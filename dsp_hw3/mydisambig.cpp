#include "Ngram.h"
#include <stdio.h>
#include "Vocab.h"
#include <map>
#include <vector>
#include "File.h"
#include "Prob.h"
#define MAX_LINE 50000
#define MAX_LEN 200
#define MAX_CANDIDATE 1000
#define ZERO_P -100
using namespace std;
int main(int argc, char **argv){
	int order = 2;
	Vocab voc;
	Vocab Z,B;
	FILE *outfile = fopen(argv[4],"w");

	/* read map */
	//VocabMap map(Z,B);
	map<VocabIndex,vector<VocabIndex>> m;

	File mapfile(argv[2],"r");
	char *buf;
	while(buf = mapfile.getline()){
		VocabString tmp[MAX_LINE];
		unsigned int len = Vocab::parseWords(buf,tmp,MAX_LINE);
		if(Z.getIndex(tmp[0]) == Vocab_None){
			VocabIndex key = Z.addWord(tmp[0]); // add to Zhu-In
			for(int i = 1;i<len;i++){
				VocabIndex value = B.addWord(tmp[i]);
				m[key].push_back(value);
			}
		}
	}
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
		Prob p; // probability
		VocabIndex empty_context[] = {Vocab_None};
		VocabIndex bigram_context[] = {Vocab_None,Vocab_None};
		LogP viterbiP[MAX_LEN][MAX_CANDIDATE]; // viterbi probability
		VocabIndex graph[MAX_LEN][MAX_CANDIDATE]; // viterbi graph
		int Backtrack[MAX_LEN][MAX_CANDIDATE];
		int candidate_num[MAX_LEN];
		VocabString test[MAX_LINE];
		unsigned int numOfwords = Vocab::parseWords(str,test,MAX_LINE);
		/* initialize viterbi*/
		int numOfCan = 0;
		VocabIndex Key = Z.getIndex(test[0]);
		int len = m[Key].size();
		for(int i = 0;i<len;i++){
			VocabIndex tmpCan = voc.getIndex(B.getWord(m[Key][i]));
			tmpCan = (tmpCan == Vocab_None) ? voc.getIndex(Vocab_Unknown) : tmpCan;
			LogP tmpP = lm.wordProb(tmpCan,empty_context);
			viterbiP[0][numOfCan] = (tmpP == LogP_Zero) ? ZERO_P : tmpP;
			graph[0][numOfCan] = m[Key][i];
			Backtrack[0][numOfCan] = -1;
			numOfCan++;
		}
		candidate_num[0] = numOfCan;
	
		/* recursion of viterbi */
		for(int i = 1;i<numOfwords;i++){
			Key = Z.getIndex(test[i]);
			numOfCan = 0;
			len = m[Key].size();
			for(int s = 0;s < len;s++){
				VocabIndex tmpCan = voc.getIndex(B.getWord(m[Key][s]));
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
				graph[i][numOfCan] = m[Key][s];
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
		fprintf(outfile,"<s> ");
		for(int i = 0;i<numOfwords;i++){
			fprintf(outfile,"%s ",ans[i]);
		}
		fprintf(outfile,"</s>\n");
	}
	testfile.close();
	return 0;
}
