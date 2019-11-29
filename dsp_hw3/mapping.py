import sys
import re 

def main(argv):
    inputfile = open(argv[0],"r",encoding='big5-hkscs')
    outfile = open(argv[1],"w",encoding='big5-hkscs')
    maps = [[] for _ in range(37)]
    line = inputfile.readline()
    while line:
        line = line[:-1:]
        B_Z = re.split(" |/",line);
        for i in B_Z[1:]:
            maps[int(ord(i[0])) - int(ord('ㄅ'))].append(B_Z[0])
        line = inputfile.readline()
    for i,item in enumerate(maps):
        if(len(item) != 0):
            outfile.write(chr(int(ord('ㄅ')+i)))
            outfile.write(' ')
        for j in item:
            outfile.write(j)
            if(j == item[-1]): 
                outfile.write('\n')
            else:
                outfile.write(' ')
        for j in item:
            outfile.write(j + ' ' + j + '\n')
if __name__ == "__main__":
    main(sys.argv[1:])
