#include "Ngram.h"
#include <stdio.h>
#include "Vocab.h"
#include "VocabMap.h"
#include "File.h"
#include "Prob.h"
#define MAX_LINE 50000
#define MAX_LEN 200
#define MAX_CANDIDATE 1000
#define ZERO_P -100
int main(int argc, char **argv){
	int order = 2;
	Vocab voc;
	Vocab Z,B;
	FILE *outfile = fopen(argv[4],"w");

	/* read map */
	VocabMap map(Z,B);
	File mapfile(argv[2],"r");
	map.read(mapfile);
	mapfile.close();

	/* read language model */	
	Ngram lm(voc,order);
	File lmfile(argv[3],"r");
	lm.read(lmfile);
	lmfile.close();
	
	/* read test data */
	File testfile(argv[1],"r");
	char *str;
	while(str = testfile.getline()){
		VocabIndex value; // map value
		Prob p; // probability
		VocabIndex empty_context[] = {Vocab_None};
		VocabIndex bigram_context[] = {Vocab_None,Vocab_None};
		LogP viterbiP[MAX_LEN][MAX_CANDIDATE]; // viterbi probability
		VocabIndex graph[MAX_LEN][MAX_CANDIDATE]; // viterbi graph
		int Backtrack[MAX_LEN][MAX_CANDIDATE];
		int candidate_num[MAX_LEN];
		VocabString test[MAX_LINE];
		unsigned int numOfwords = Vocab::parseWords(str,&(test[1]),MAX_LINE);
		test[0] = "<s>";
		test[numOfwords+1] = "</s>";
		numOfwords += 2;
		/* initialize viterbi*/
		VocabMapIter iter(map,Z.getIndex(test[0]));
		iter.init();
		int numOfCan = 0;
		while(iter.next(value,p)){
			VocabIndex tmpCan = voc.getIndex(B.getWord(value));
			tmpCan = (tmpCan == Vocab_None) ? voc.getIndex(Vocab_Unknown) : tmpCan;
			LogP tmpP = lm.wordProb(tmpCan,empty_context);
			viterbiP[0][numOfCan] = (tmpP == LogP_Zero) ? ZERO_P : tmpP;
			graph[0][numOfCan] = value;
			Backtrack[0][numOfCan] = -1;
			numOfCan++;
		}
		candidate_num[0] = numOfCan;
	
		/* recursion of viterbi */
		for(int i = 1;i<numOfwords;i++){
			VocabMapIter iter(map,Z.getIndex(test[i]));
			iter.init();
			numOfCan = 0;
			while(iter.next(value,p)){
				VocabIndex tmpCan = voc.getIndex(B.getWord(value));
				tmpCan = (tmpCan == Vocab_None) ? voc.getIndex(Vocab_Unknown) : tmpCan;
				LogP maxP = LogP_Zero;
				for(int j = 0;j<candidate_num[i-1];j++){
					VocabIndex lastv = voc.getIndex(B.getWord(graph[i-1][j]));
					lastv = (lastv == Vocab_None) ? voc.getIndex(Vocab_Unknown) : lastv;
					bigram_context[0] = lastv;
					LogP tmp = lm.wordProb(tmpCan,bigram_context);
					LogP uniP = lm.wordProb(tmpCan,empty_context);
					if(tmp == LogP_Zero && uniP == LogP_Zero) tmp = ZERO_P;
					tmp += viterbiP[i-1][j];
					if(tmp > maxP){
						maxP = tmp;
						Backtrack[i][numOfCan] = j;
					}
				}
				viterbiP[i][numOfCan] = maxP;
				graph[i][numOfCan] = value;
				numOfCan++;
			}
			candidate_num[i] = numOfCan;
		}

		/* find max path */
		LogP total_max = LogP_Zero;
		int max_index = -1;
		for(int i = 0;i<candidate_num[numOfwords-1];i++){
			if(viterbiP[numOfwords-1][i] > total_max){
				total_max = viterbiP[numOfwords-1][i];
				max_index = i;
			}
		}
		VocabString ans[MAX_LINE];
		for(int i = numOfwords-1;i>=0;i--){
			ans[i] = B.getWord(graph[i][max_index]);
			max_index = Backtrack[i][max_index];
		}
		
		/* write to output */
		for(int i = 0;i<numOfwords;i++){
			fprintf(outfile,"%s%s",ans[i],(i == numOfwords-1) ? "\n":" ");
		}
	}
	testfile.close();
	return 0;
}
