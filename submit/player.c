#include <stdio.h>
#include <stdlib.h>
#include <my-ipc.h>
#include <client-side.h>
#include <redundant.h>
#include <public.h>

const char myName[] = "2959304354";
const char deployment1[] = "Ba3a4a5a6 Ce8f8g8 Cc0d0e0 Dc3c4 Dg2g3 Di3i4 Sc7 Se2 Se5 Sh6 ";
const char deployment2[] = "Bd8e8f8g8 Ca2a3a4 Ce4e5e6 Dc0d0 Dc5c6 Di4i5 Sf0 Sh1 Sg3 Sg6 ";
const int len_s = 1;
const int len_d = 2;
const int len_c = 3;
const int len_b = 4;
int root_x, root_y;
int rooting = -1;//1 = true, -1 = false;
int root_count = 0;
int pre_ship_len = 0;
int out_exist = 0;//1 = true, 0=false

enum ship {
  UNKNOWN,
  ROCK,
  NOSHIP,
  BSHIP,
  CSHIP,
  DSHIP,
  SSHIP,
  BSHIP_BEFORE,
  CSHIP_BEFORE,
  DSHIP_BEFORE,
  SSHIP_BEFORE
};
//現段階では方向のとり方が少し間違っている
enum direction {
  LEFT,
  RIGHT,
  UP,
  DOWN
};

int cur_x,cur_y;
enum ship enemy_board[BD_SIZE][BD_SIZE];    // BD_SIZE is 9 (defined in public.h)
enum direction pre_direction;

void respond_with_name(void)
{
  char *str = (char *)malloc(sizeof(myName));
  strcpy(str, myName);
  send_to_ref(str);
  free(str);
}

void respond_with_deployment(void)
{
  if(out_exist == 0){
    char *str = (char *)malloc(sizeof(deployment1));
    strcpy(str, deployment1);
    send_to_ref(str);
    free(str);
  }else{
    char *str = (char *)malloc(sizeof(deployment2));
    strcpy(str, deployment2);
    send_to_ref(str);
    free(str);
  }
}

void quit_root(void){
  rooting = -1;
  root_count = 0;
}

void set_noship(int x, int y){
  if(x>=0&&x<BD_SIZE&&y>=0&&y<BD_SIZE){
     enemy_board[x][y] = NOSHIP;
  }
}

void set_noship_cross(int x, int y){
  set_noship(x-1,y-1);
  set_noship(x-1,y+1);
  set_noship(x+1,y-1);
  set_noship(x+1,y+1);
}

void write_output(void){
  if(out_exist == 0){
    remove("2959304354_output.txt");
    FILE *fp = fopen("2959304354_output.txt", "w");
    for(int i=0;i<BD_SIZE;i++){
      for(int j=0;j<BD_SIZE;j++){
        fprintf(fp, "%d", enemy_board[i][j]);
      }
    }
    fclose(fp);
  }
}

void init_board(void)
{
  //======kokokara======
  int ix, iy;

  for(ix = 0; ix < (BD_SIZE); ix++)
  {
    for(iy = 0; iy < (BD_SIZE); iy++)
    {
      //======kokokara======
	enemy_board[ix][iy] = UNKNOWN;
    }
  }

  //rock is out of bound

  enemy_board[0][0] = NOSHIP;

  //======kokokara======
  enemy_board[0][1] = NOSHIP;
  enemy_board[0][1] = NOSHIP;
  enemy_board[1][0] = NOSHIP;
  enemy_board[8][8] = NOSHIP;
  enemy_board[8][7] = NOSHIP;
  enemy_board[7][8] = NOSHIP;
  enemy_board[0][8] = NOSHIP;
  enemy_board[0][7] = NOSHIP;
  enemy_board[1][8] = NOSHIP;
  enemy_board[8][0] = NOSHIP;
  enemy_board[8][1] = NOSHIP;
  enemy_board[7][0] = NOSHIP;
  //======kokomade======
  if(out_exist == 1){
   //enemy_ship_boardに書き込む
    char str[90];
    FILE *fp = fopen("2959304354_output.txt", "r");
    if(fp != NULL){
      fgets(str, 81, fp);
      for(int i=0;i<BD_SIZE;i++){
        for(int j=0;j<BD_SIZE;j++){
          switch(str[i*BD_SIZE + j] - '0'){
            case 0:
              enemy_board[i][j] = UNKNOWN;
              break;
            case 1:
              enemy_board[i][j] = NOSHIP;
              break;
            case 2:
              enemy_board[i][j] = NOSHIP;
              break;
            case 3:
              enemy_board[i][j] = BSHIP_BEFORE;
              break;
            case 4:
              enemy_board[i][j] = CSHIP_BEFORE;
              break;
            case 5:
              enemy_board[i][j] = DSHIP_BEFORE;
              break;
            case 6:
              enemy_board[i][j] = SSHIP_BEFORE;
              break;
            default:
              enemy_board[i][j] = UNKNOWN;
          }
        }
      }
    }
  }
}

int *get_unknown_around(int x, int y){
  int *unknown = malloc(sizeof(int) * 2);
  if(x-1>=0){
    if(enemy_board[x-1][y] == UNKNOWN){rooting=1;unknown[0]=x-1;unknown[1]=y;pre_direction = LEFT;return unknown;}
  }
  if(x+1<=8){
    if(enemy_board[x+1][y] == UNKNOWN){rooting=1;unknown[0]=x+1;unknown[1]=y;pre_direction = RIGHT;return unknown;}
  }
  if(y-1>=0){
    if(enemy_board[x][y-1] == UNKNOWN){rooting=1;unknown[0]=x;unknown[1]=y-1;pre_direction = DOWN;return unknown;}
  }
  if(y+1<=8){
    if(enemy_board[x][y+1] == UNKNOWN){rooting=1;unknown[0]=x;unknown[1]=y+1;pre_direction = UP;return unknown;}
  }
  if(pre_ship_len>0&&root_count < pre_ship_len){
    switch(pre_direction){
      case LEFT:
        unknown[0] = root_x + root_count;
        unknown[1] = root_y; 
        break;
      case RIGHT:
        unknown[0] = root_x - root_count;
        unknown[1] = root_y;
        break;
      case UP:
        unknown[0] = root_x;
        unknown[1] = root_y - root_count;
        break;
      case DOWN:
        unknown[0] = root_x;
        unknown[1] = root_y + root_count;
        break;
      default:
        unknown[0] = 5;
        unknown[1] = 5;
        break;
    }
    
  }else{
    quit_root();
    unknown[0]=1; unknown[1]=1;
  }
  if(unknown[0] < 0 || unknown[0] > 8 || unknown[1] < 0 || unknown[1] > 8){
    quit_root();
    unknown[0] = 1; unknown[1]=1;
  }
  return unknown;
} 

void respond_with_shot(void)
{
  char shot_string[MSG_LEN];
  int x, y;
  int *root_around;
  while (TRUE)
  {
    int flag = 0;
    if(out_exist == 1){
      for(int i=0;i<BD_SIZE;i++){
        for(int j=0;j<BD_SIZE;j++){
          if(enemy_board[i][j] == BSHIP_BEFORE || enemy_board[i][j] == CSHIP_BEFORE || enemy_board[i][j] == DSHIP_BEFORE || enemy_board[i][j] == SSHIP_BEFORE){
            x = i;
            y = j;
            i = BD_SIZE - 1;
            j = BD_SIZE - 1;
            flag = 1;
          }
        }
      }
    }
    if(flag == 1){ break; }
    
    if(rooting == 1){
      root_around = get_unknown_around(root_x, root_y);
      if(rooting == 1 && root_around[0] >= 0 && root_around[1] >= 0){
        x = root_around[0];
        y = root_around[1];
        break;
      }
    }
    x = rand() % BD_SIZE;
    y = rand() % BD_SIZE;
    if(x<0 || x > 8 || y<0 || y>8){
      continue;
    }
    if((enemy_board[x][y] == UNKNOWN))
    { 
      break; 
    }
  }
  if(x<0 || x > 8 || y<0 || y>8){
    respond_with_shot();
    return;
  }
  printf("[%s] shooting at %d%d ... ", myName, x, y);
  sprintf(shot_string, "%d%d", x, y);
  send_to_ref(shot_string);
  cur_x = x;
  cur_y = y;
}

void set_root(int x, int y){
    root_x = x;
    root_y = y;
    rooting = 1;
}

void record_result(int x,int y,char line[])
{
  if(line[13]=='B')
  {
    enemy_board[x][y] = BSHIP;
    set_noship_cross(x,y);
    set_root(x,y);
    if(rooting){
      root_count ++;
      if(root_count >= len_b){quit_root();}
    }
    pre_ship_len = len_b;
  }
  else if(line[13]=='C')
  {
    //====kokokara====
     enemy_board[x][y] = CSHIP;
     set_noship_cross(x,y);
     set_root(x,y);
     if(rooting){
      root_count ++;
      if(root_count >= len_c){quit_root();}
    }
    pre_ship_len = len_c;
    //====kokomade====
  }
  else if(line[13]=='D')
  {
    //====kokokara====
     enemy_board[x][y] = DSHIP;
     set_noship_cross(x,y);
     set_root(x,y);
     if(rooting){
      root_count ++;
      if(root_count >= len_d){quit_root();}
    }
    pre_ship_len = len_d;
    //====kokomade====
  }
  else if(line[13]=='S')
  {
    //====kokokara====
    enemy_board[x][y] = SSHIP;
    if(x-1>=0){enemy_board[x-1][y] = NOSHIP;}
    if(x+1<=8){enemy_board[x+1][y] = NOSHIP;}
    if(y-1>=0){enemy_board[x][y-1] = NOSHIP;}
    if(y+1<=8){enemy_board[x][y+1] = NOSHIP;}
    set_noship_cross(x,y);
    //====kokomade====
    pre_ship_len = len_s;
  }
  else if(line[13]=='R')
  {
    enemy_board[x][y] = NOSHIP;
    
    pre_ship_len = 0;
  }
  else
  {
    //====kokokara====
    enemy_board[x][y] = NOSHIP;
    //====kokomade====
    pre_ship_len = 0;
  }
  write_output();
}

void print_board(void){
  int ix, iy;

  for (iy = BD_SIZE - 1; iy >= 0; iy--)
  {
    printf("%2d ", iy);
    for (ix = 0; ix < BD_SIZE; ix++)
    {
      switch(enemy_board[ix][iy])
      {
        case UNKNOWN:
          printf("U ");
          break;
        case NOSHIP:
          printf("N ");
          break;
        case ROCK:
          printf("R ");
          break;
        case BSHIP:
          printf("B ");
          break;
        case CSHIP:
          printf("C ");
          break;
        case DSHIP:
          printf("D ");
          break;
        case SSHIP:
          printf("S ");
          break;
        case BSHIP_BEFORE:
          printf("W ");
          break;
        case CSHIP_BEFORE:
          printf("X ");
          break;
        case DSHIP_BEFORE:
          printf("Y ");
          break;
        case SSHIP_BEFORE:
          printf("Z ");
          break;
        default:
          break;
      }
    }
    printf("\n");
  }

  printf("  ");
  for (ix = 0; ix < BD_SIZE; ix++)
  {
    printf("%2d", ix);
  }
  printf("\n\n");
}

void handle_messages(void)
{
  char line[MSG_LEN];

  srand(getpid());
  init_board();
  
  while (TRUE)
  {
    receive_from_ref(line);

    if(message_has_type(line, "name?"))
    {
      respond_with_name(); 
    }
    else if(message_has_type(line, "deployment?"))
    {
       respond_with_deployment(); 
    }
    else if(message_has_type(line, "shot?"))
    {
      respond_with_shot(); 
    }
    else if(message_has_type(line, "shot-result:"))
    {
      record_result(cur_x,cur_y,line);
      printf("[%s] result: %c\n", myName, line[13]);
      print_board();
    }
    else if(message_has_type(line, "end:"))
    {
      break;
    }
    else
    {
      printf("[%s] ignoring message: %s", myName, line);
    }
  }
}

int main()
{
  FILE *fp = fopen("2959304354_output.txt", "r");
  if (fp == NULL) {
    out_exist = 0;//出力ファイルが存在しない
  }else{
    out_exist = 1;//出力ファイルが存在
    fclose(fp);
  }
  client_make_connection();
  handle_messages();
  client_close_connection();
  if(out_exist == 1){
    remove("2959304354_output.txt");
  }
  return 0;
}
