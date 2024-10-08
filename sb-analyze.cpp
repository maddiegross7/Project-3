//Maddie Gross
//Lab 3
//finds all of the sets that would result in a score given a specific board

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include <iostream>
#include <vector>
#include "disjoint.h"
using namespace std;

#define talloc(type, num) (type *) malloc(sizeof(type)*(num))

class Superball {
  public:
    Superball(int argc, char **argv);
    int r;
    int c;
    int mss;
    int empty;
    vector <int> board;
    vector <int> goals;
    vector <int> colors;
    DisjointSetByRankWPC *ds;
    void analyzeSuperball();
};

//This is the info I wanted to know about the set so that I could 
//determine if they were good enough to score
class ScoringSet{
  public:
    int setName = -1;
    int size = 0;
    int character;
    pair<int, int> score = make_pair(NULL,NULL);
};

void usage(const char *s) 
{
  fprintf(stderr, "usage: sb-play rows cols min-score-size colors\n");
  if (s != NULL) fprintf(stderr, "%s\n", s);
  exit(1);
}

Superball::Superball(int argc, char **argv)
{
  int i, j;
  string s;


  if (argc != 5) usage(NULL);

  if (sscanf(argv[1], "%d", &r) == 0 || r <= 0) usage("Bad rows");
  if (sscanf(argv[2], "%d", &c) == 0 || c <= 0) usage("Bad cols");
  if (sscanf(argv[3], "%d", &mss) == 0 || mss <= 0) usage("Bad min-score-size");

  colors.resize(256, 0);

  for (i = 0; i < strlen(argv[4]); i++) {
    if (!isalpha(argv[4][i])) usage("Colors must be distinct letters");
    if (!islower(argv[4][i])) usage("Colors must be lowercase letters");
    if (colors[argv[4][i]] != 0) usage("Duplicate color");
    colors[argv[4][i]] = 2+i;
    colors[toupper(argv[4][i])] = 2+i;
  }

  board.resize(r*c);
  goals.resize(r*c, 0);

  empty = 0;

  for (i = 0; i < r; i++) {
    if (!(cin >> s)) {
      fprintf(stderr, "Bad board: not enough rows on standard input\n");
      exit(1);
    }
    if (s.size() != c) {
      fprintf(stderr, "Bad board on row %d - wrong number of characters.\n", i);
      exit(1);
    }
    for (j = 0; j < c; j++) {
      if (s[j] != '*' && s[j] != '.' && colors[s[j]] == 0) {
        fprintf(stderr, "Bad board row %d - bad character %c.\n", i, s[j]);
        exit(1);
      }
      board[i*c+j] = s[j];
      if (board[i*c+j] == '.') empty++;
      if (board[i*c+j] == '*') empty++;
      if (isupper(board[i*c+j]) || board[i*c+j] == '*') {
        goals[i*c+j] = 1;
        board[i*c+j] = tolower(board[i*c+j]);
      }
    }
  }

  ds = new DisjointSetByRankWPC(r * c);
}

void Superball::analyzeSuperball(){

  //creating the sets checking the block to the right and the block below, 
  //not checking if at the bottom row or on the right side because those would not touch the next one
  for(int i = 0; i < r*c; i++){
    if((i+1)%(c) != 0 && colors[board[i]] == colors[board[i+1]] && colors[board[i]] != 0){
      int current = ds->Find(i);
      int under = ds->Find(i+1);
      if(current != under){
        ds->Union(current, under);
      }
    }
    if(((i + c) < board.size()) && colors[board[i]] == colors[board[i+c]] && colors[board[i]] != 0){
      int current = ds->Find(i);
      int right = ds->Find(i+c);
      if(current != right){
        ds->Union( current, right);
      }
    }
  }

  vector<ScoringSet> sets;

  //making the sets based on the sets that I have to get the needed info
  for(int i = 0; i < r*c; i++){
    int block = ds->Find(i);
    bool found = false;
    for(auto &set: sets){
      if(set.setName == block){
        set.size += 1;
        found = true;
        if(goals[i] == 1){
          set.score = make_pair(int(i/c), i%c);
        }
        break;
      }
    }

    if(!found && board[i] != 42){
      ScoringSet thisSet;
      thisSet.setName = block;
      thisSet.size = 1;
      thisSet.character = board[i];
      if(goals[i] == 1){
          thisSet.score = make_pair(int(i/c), i%c);
        }
      sets.push_back(thisSet);
    }
  }

  //printing out the sets that would result in a score
  cout << "Scoring sets:" << endl;

  for(int i = 0; i < sets.size(); i++ ){
    if(mss <= sets[i].size  && (sets[i].score.first != NULL || sets[i].score.second != NULL)){
      cout << "  Size: " << sets[i].size << " Char: " << (char)(sets[i].character) <<
        " Scoring Cell: " << sets[i].score.first << "," << sets[i].score.second << endl;
    }
  }
}



main(int argc, char **argv)
{
  Superball *s;

  int i, j;
 
  s = new Superball(argc, argv);

  // for(i=0; i<s->r*s->c; i++){
  //   cout << s->goals[i] << " " ;
  //   if((i+1)%s->c == 0){
  //     cout << endl;
  //   }
  // }
  
  s->analyzeSuperball();

}
