#include <cstdio>
//Maddie Gross
//Lab 3
//finds the move that could score the most points for superball

#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
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
    void findSwap();
};

//This is the info I wanted to know about the set so that I could 
//determine if they were good enough to score
class ScoringSet{
  public:
    int setName = -1;
    int size = 0;
    int character;
    int pointValue = 0;
    pair<int, int> score = make_pair(NULL,NULL);
    vector<pair<int, int>> blocksInSet;
};

//I wanted more details so I made this so I could look at a block if I wanted to 
class Block{
  public:
    int setName = -1;
    int value = 0;
    pair<int, int> position = make_pair(NULL, NULL);
    pair<int, int> distanceToGoal;
};

//I dont think I used these much just to get maybe the scoring/goal block idrk its late and I am tired
class Score {
  public:
    int x;
    int y;
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

  ifstream fileIn("tmp.superball");
  cin.rdbuf(fileIn.rdbuf());

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

double getDistance(int x1, int y1){
  return sqrt(((x1 * x1)) + ((y1* y1)));
}

void Superball::analyzeSuperball(){
  //finding those scoring/goal blocks
  vector<Score> scores;
  for(int i = 0; i < r*c; i++){
    if(goals[i] == 1){
      Score goal;
      goal.x = i%c;
      goal.y =  int(i/c);
      scores.push_back(goal);
    }
  }
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
  vector<Block> blocks;

  //making the sets based on the sets that I have to get the needed info
  for(int i = 0; i < r*c; i++){
    if(colors[board[i]] > 0){
      int block = ds->Find(i);
      bool found = false;
      for(auto &set: sets){
        if(set.setName == block){
          set.size += 1;
          found = true;
          set.pointValue += colors[board[i]];
          if(goals[i] == 1){
            set.score = make_pair(i%c,int(i/c));
          }
          set.blocksInSet.push_back(make_pair(i%c, int(i/c)));
          break;
        }
      }

      if(!found){
        ScoringSet thisSet;
        thisSet.setName = block;
        thisSet.size = 1;
        thisSet.character = board[i];
        thisSet.pointValue = colors[board[i]];
        if(goals[i] == 1){
            thisSet.score = make_pair(i%c, int(i/c));
          }
        thisSet.blocksInSet.push_back(make_pair(i%c, int(i/c)));
        sets.push_back(thisSet);
      }

      if(colors[board[i]] != 0){
        Block thisBlock;
        thisBlock.value = colors[board[i]];
        thisBlock.position = make_pair(i%c, int(i/c));
        thisBlock.setName = ds->Find(i);
        thisBlock.distanceToGoal = make_pair(c, r);
        if(goals[i] == 1){
          thisBlock.distanceToGoal = make_pair(0, 0);
        }else{
          for(int j = 0; j < scores.size(); j++){
            int xDiff = scores[j].x - thisBlock.position.first;
            int yDiff = scores[j].y - thisBlock.position.second;
            if(getDistance(xDiff, yDiff) < getDistance(thisBlock.distanceToGoal.first, thisBlock.distanceToGoal.second)){
              thisBlock.distanceToGoal.first = xDiff;
              thisBlock.distanceToGoal.second = yDiff;
            }
          }
        }
        blocks.push_back(thisBlock);
      }
    }
  }
  vector<ScoringSet> scoringSets;
  
  for(int i = 0; i < sets.size(); i++ ){
    if(mss <= sets[i].size && (sets[i].score.first != NULL || sets[i].score.second != NULL)){
      scoringSets.push_back(sets[i]);
    } 
  }
  // I do this because if I have less than 10 I want to start trying to score to get less 
  if(empty <= 10 && scoringSets.size() == 0) {
    cout << "SWAP " << blocks[1].position.second << " " << blocks[1].position.first <<
         " " << blocks[2].position.second << " " << blocks[2].position.first << endl;

  }else if(empty <= 10){
    ScoringSet* highest = &scoringSets[0];
    for(int i=1; i<scoringSets.size(); i++){
      if(scoringSets[i].pointValue > highest->pointValue){
        highest = &scoringSets[i];
      }
    }
    cout << "SCORE " << highest->score.second << " " << highest->score.first << endl;
  }else{
    //if I have room to make moves I want to find the best set to try to keep making bigger
    ScoringSet* highest = &sets[0];
    if(scoringSets.size() > 0){
      highest = &scoringSets[0];
      for(int i=1; i<scoringSets.size(); i++){
        if(scoringSets[i].pointValue > highest->pointValue){
          highest = &scoringSets[i];
        }
      }
    }else{
      for(int i=1; i<sets.size(); i++){
        if(sets[i].pointValue > highest->pointValue || 
          (sets[i].pointValue == highest->pointValue && sets[i].size > highest->size)){
            highest = &sets[i];
        }
      }
    }

    //so this one I am trying to find the set of the same color that is least helpful toward scoring
    ScoringSet* lowest = &sets[0];
    for(int i = 1; i < sets.size(); i++){
      if((sets[i].character == highest->character && 
        (lowest->character != highest->character || sets[i].pointValue < lowest->pointValue))
         ){
        lowest = &sets[i];
      }
    }

    //this I try to find a block near the set that I can seap the value with the lowest block that I chose
    Block& swap = blocks[0];
      for(int i = 0; i < highest->blocksInSet.size(); i++){
        int index = (highest->blocksInSet[i].second*c) + highest->blocksInSet[i].first;
        if(index%c != 0 && board[index-1] != highest->character && colors[board[index-1]] > 0){
          cout << "SWAP " << highest->blocksInSet[i].second << " " << (highest->blocksInSet[i].first-1) <<
         " " << lowest->blocksInSet[0].second << " " << lowest->blocksInSet[0].first << endl;
        }
        if(index+c < r*c && board[index+c] != highest->character && colors[board[index+c]] > 0){
          cout << "SWAP " << (highest->blocksInSet[i].second+1) << " " << highest->blocksInSet[i].first <<
         " " << lowest->blocksInSet[0].second << " " << lowest->blocksInSet[0].first << endl;
        }
        if(((index+1)%c != 0) && board[index+1] != highest->character && colors[board[index+1]] > 0){
          cout << "SWAP " << highest->blocksInSet[i].second << " " << (highest->blocksInSet[i].first+1) <<
         " " << lowest->blocksInSet[0].second << " " << lowest->blocksInSet[0].first << endl;
        }
        if(index-c > 0 && board[index-c] != highest->character && colors[board[index-c]] > 0){
          cout << "SWAP " << (highest->blocksInSet[i].second-1) << " " << highest->blocksInSet[i].first <<
         " " << lowest->blocksInSet[0].second << " " << lowest->blocksInSet[0].first << endl;
        }
        else{
          for(int i = 1; i < blocks.size(); i++){
          if(blocks[i].value != colors[lowest->character] && 
            (getDistance(blocks[i].distanceToGoal.first, blocks[i].distanceToGoal.second) < 
            getDistance(swap.distanceToGoal.first, swap.distanceToGoal.second))){
              swap = blocks[i];
            }
        }
      }
    }    
    cout << "SWAP " << lowest->blocksInSet[0].second << " " << lowest->blocksInSet[0].first <<
         " " << swap.position.second << " " << swap.position.first << endl;
  }
}


main(int argc, char **argv)
{
  Superball *s;
  int i, j;

  s = new Superball(argc, argv);
  
  s->analyzeSuperball();
}
