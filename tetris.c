#include "tetris.h"

static struct sigaction act, oact;

int main(){
	int exit=0;

	initscr();
	noecho();
	keypad(stdscr, TRUE);	

	srand((unsigned int)time(NULL));

    createRankList();
    
	while(!exit){
		clear();
		switch(menu()){
		case MENU_PLAY: play(); break;
		case MENU_EXIT: exit=1; break;
        case MENU_RANK: rank(); break;
        case MENU_REC_PLAY: recommendedPlay(); break;
		default: break;
		}
	}
    
    //프로그램 종료 전 메모리 해제
    Node *ptr=rankHead;
    for(int i=0; i<rankNum; i++){
        rankHead=ptr->link;
        free(ptr);
        ptr=rankHead;
    }

	endwin();
	system("clear");
	return 0;
}

void InitTetris(){
	int i,j;

    recRoot = (RecNode*)malloc(sizeof(RecNode));
    recRoot->f=(char(*)[WIDTH])malloc(sizeof(char)*HEIGHT*WIDTH);
    recRoot->lv = -1;
    recRoot->score = 0;
    
	for(j=0;j<HEIGHT;j++)
		for(i=0;i<WIDTH;i++){
            field[j][i]=0;
            recRoot->f[j][i]=0;
        }

    for(i=0; i<VISIBLE_BLOCKS ; i++)
        nextBlock[i]=rand()%7;
    
	blockRotate=0;
	blockY=-1;
	blockX=WIDTH/2-2;
	score=0;	
	gameOver=0;
	timed_out=0;
    num_of_nodes=1;
    //recommend(recRoot);
    modified_recommend(recRoot);
    free(recRoot->f);
    free(recRoot);
    
	DrawOutline();
	DrawField();
	DrawBlockWithFeatures(blockY,blockX,nextBlock[0],blockRotate);
	DrawNextBlock(nextBlock);
	PrintScore(score);
    
}

void DrawOutline(){	
	int i,j;
	/* 블럭이 떨어지는 공간의 태두리를 그린다.*/
	DrawBox(0,0,HEIGHT,WIDTH);

	/* next block을 보여주는 공간의 태두리를 그린다.*/
	move(2,WIDTH+10);
	printw("NEXT BLOCK");
	DrawBox(3,WIDTH+10,4,8);
    DrawBox(9,WIDTH+10,4,8);

	/* score를 보여주는 공간의 태두리를 그린다.*/
	move(15,WIDTH+10);
	printw("SCORE");
	DrawBox(16,WIDTH+10,1,8);
}

int GetCommand(){
	int command;
	command = wgetch(stdscr);
	switch(command){
	case KEY_UP:
		break;
	case KEY_DOWN:
		break;
	case KEY_LEFT:
		break;
	case KEY_RIGHT:
		break;
	case ' ':	/* space key*/
		/*fall block*/
		break;
	case 'q':
	case 'Q':
		command = QUIT;
		break;
	default:
		command = NOTHING;
		break;
	}
	return command;
}

int ProcessCommand(int command){
	int ret=1;
	int drawFlag=0;
	switch(command){
	case QUIT:
		ret = QUIT;
		break;
	case KEY_UP:
		if((drawFlag = CheckToMove(field,nextBlock[0],(blockRotate+1)%4,blockY,blockX)))
			blockRotate=(blockRotate+1)%4;
		break;
	case KEY_DOWN:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY+1,blockX)))
			blockY++;
		break;
	case KEY_RIGHT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX+1)))
			blockX++;
		break;
	case KEY_LEFT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX-1)))
			blockX--;
		break;
	default:
		break;
	}
	if(drawFlag) DrawChange(field,command,nextBlock[0],blockRotate,blockY,blockX);
	return ret;	
}

void DrawField(){
	int i,j;
	for(j=0;j<HEIGHT;j++){
		move(j+1,1);
		for(i=0;i<WIDTH;i++){
			if(field[j][i]==1){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(".");
		}
	}
}


void PrintScore(int score){
	move(17,WIDTH+11);
	printw("%8d",score);
}

void DrawNextBlock(int *nextBlock){
	int i, j;
    
    if(VISIBLE_BLOCKS>=2){
        for( i = 0; i < 4; i++ ){
            move(4+i,WIDTH+13);
            for( j = 0; j < 4; j++ ){
                if( block[nextBlock[1]][0][i][j] == 1 ){
                    attron(A_REVERSE);
                    printw(" ");
                    attroff(A_REVERSE);
                }
                else printw(" ");
            }
        }
    }
    
    if(VISIBLE_BLOCKS>=3){
        for( i = 0; i < 4; i++ ){
            move(10+i,WIDTH+13);
            for( j = 0; j < 4; j++ ){
                if( block[nextBlock[2]][0][i][j] == 1 ){
                    attron(A_REVERSE);
                    printw(" ");
                    attroff(A_REVERSE);
                }
                else printw(" ");
            }
        }
    }
}

void DrawBlock(int y, int x, int blockID,int blockRotate,char tile){
	int i,j;
	for(i=0;i<4;i++)
		for(j=0;j<4;j++){
			if(block[blockID][blockRotate][i][j]==1 && i+y>=0){
				move(i+y+1,j+x+1);
				attron(A_REVERSE);
				printw("%c",tile);
				attroff(A_REVERSE);
			}
		}

	move(HEIGHT,WIDTH+10);
}

void DrawBox(int y,int x, int height, int width){
	int i,j;
	move(y,x);
	addch(ACS_ULCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_URCORNER);
	for(j=0;j<height;j++){
		move(y+j+1,x);
		addch(ACS_VLINE);
		move(y+j+1,x+width+1);
		addch(ACS_VLINE);
	}
	move(y+j+1,x);
	addch(ACS_LLCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_LRCORNER);
}

void play(){
	int command;
	clear();
	act.sa_handler = BlockDown;
	sigaction(SIGALRM,&act,&oact);
	InitTetris();
	do{
		if(timed_out==0){
			alarm(1);
			timed_out=1;
		}

		command = GetCommand();
		if(ProcessCommand(command)==QUIT){
			alarm(0);
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
			move(HEIGHT/2,WIDTH/2-4);
			printw("Good-bye!!");
			refresh();
			getch();

			return;
		}
	}while(!gameOver);

	alarm(0);
	getch();
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
	move(HEIGHT/2,WIDTH/2-4);
	printw("GameOver!!");
	refresh();
	getch();
	newRank(score);
}

char menu(){
	printw("1. play\n");
	printw("2. rank\n");
	printw("3. recommended play\n");
	printw("4. exit\n");
	return wgetch(stdscr);
}

/////////////////////////첫주차 실습에서 구현해야 할 ㅇ함수/////////////////////////

int CheckToMove(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	// user code
    int i, j;
    for(i=0; i<4; i++)
        for(j=0; j<4; j++){
            if (block[currentBlock][blockRotate][i][j]==1){
                if(f[blockY+i][blockX+j]==1) return 0; //블록이 와야할 자리에 이미 다른 블록이 있는 경우
                if(blockY+i>=HEIGHT) return 0;
                if(blockX+j<0) return 0;
                if(blockX+j>=WIDTH) return 0;
            }
        }
    return 1;
}

void DrawChange(char f[HEIGHT][WIDTH],int command,int currentBlock,int blockRotate, int blockY, int blockX){
	// user code

	//1. 이전 블록 정보를 찾는다. ProcessCommand의 switch문을 참조할 것
    int past_rotate=blockRotate;
    int past_blockY=blockY;
    int past_blockX=blockX;
    
    switch(command){
    case KEY_UP:
        past_rotate=(past_rotate+3)%4;
        break;
    case KEY_DOWN:
        past_blockY--;
        break;
    case KEY_RIGHT:
        past_blockX--;
        break;
    case KEY_LEFT:
        past_blockX++;
        break;
    default:
        break;
    }
    //2. 이전 블록 정보를 지운다. DrawBlock함수 참조할 것.
    //DrawBlock(past_blockY, past_blockX, currentBlock, past_rotate, '.');
    int shadow_y=past_blockY;
    while(CheckToMove(f,currentBlock,past_rotate,shadow_y+1,past_blockX)){
        shadow_y++;
    }
    int i,j;
    for(i=0;i<4;i++)
        for(j=0;j<4;j++){
            if(block[currentBlock][past_rotate][i][j]==1 && i+past_blockY>=0){
                move(i+past_blockY+1,j+past_blockX+1);
                printw(".");
                move(i+shadow_y+1,j+past_blockX+1);
                printw(".");
            }
        }
    move(HEIGHT,WIDTH+10);

    //3. 새로운 블록 정보를 그린다.
    DrawBlockWithFeatures(blockY, blockX, currentBlock, blockRotate);
}

void BlockDown(int sig){
	// user code
	//강의자료 p26-27의 플로우차트를 참고한다.
    int i, j;
    
    if(CheckToMove(field, nextBlock[0], blockRotate, blockY+1, blockX)){
        blockY++;
        DrawChange(field,KEY_DOWN,nextBlock[0],blockRotate,blockY,blockX);
    }
    else {
        if(blockY == -1) gameOver = 1;
        score+=AddBlockToField(field, nextBlock[0], blockRotate, blockY, blockX);
        score+=DeleteLine(field);
        PrintScore(score);      
        
        for(i=0; i<VISIBLE_BLOCKS-1; i++)
            nextBlock[i]=nextBlock[i+1];
        nextBlock[i]=rand()%7;
        
        blockRotate=0;
        blockY=-1;
        blockX=WIDTH/2-2;
        
        recRoot = (RecNode*)malloc(sizeof(RecNode));
        recRoot->f=(char(*)[WIDTH])malloc(sizeof(char)*HEIGHT*WIDTH);
        recRoot->lv = -1;
        recRoot->score = score;
        
        
        for(i=0;i<HEIGHT;i++) // copy field
            for(j=0;j<WIDTH;j++)
                recRoot->f[i][j]=field[i][j];
            
        recommend(recRoot);
        free(recRoot->f);
        free(recRoot);
        
        DrawNextBlock(nextBlock);
        DrawField();
        DrawBlockWithFeatures(blockY,blockX,nextBlock[0],blockRotate);
    }
    timed_out=0;
}

int AddBlockToField(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	// user code
	//Block이 추가된 영역의 필드값을 바꾼다.
    int i, j;
    int touched=0;
    
    for(i=0; i<4; i++)
        for(j=0; j<4; j++){
            if(block[currentBlock][blockRotate][i][j]==1){
                if(f[blockY+i+1][blockX+j]==1 || blockY+i+1==HEIGHT) touched++;
                f[blockY+i][blockX+j]=1;
            }
        }
    return touched * 10;
}

int DeleteLine(char f[HEIGHT][WIDTH]){
	// user code

	//1. 필드를 탐색하여, 꽉 찬 구간이 있는지 탐색한다.
    int i, j, k, l, flag, num=0;
    
    for(i=0; i<HEIGHT; i++){
        flag = 1;
        for(j=0; j<WIDTH; j++){
            if(f[i][j]==0) {
                flag=0;
                break;
            }
        }
        //2. 꽉 찬 구간이 있으면 해당 구간을 지운다. 즉, 해당 구간으로 필드값을 한칸씩 내린다.
        if(flag){
            num++;
            for(l=i; l>=0; l--){
                for(k=0; k<WIDTH; k++){
                    if(l==0) f[l][k]=0;
                    else f[l][k]=f[l-1][k];
                }
            }
        }
    }
    return num*num*100;
}

///////////////////////////////////////////////////////////////////////////

void DrawShadow(int y, int x, int blockID,int blockRotate){
	// user code
    
    int shadow_y=y;
    while(CheckToMove(field,blockID,blockRotate,shadow_y+1,x)){
        shadow_y++;
    }
    DrawBlock(shadow_y,x,blockID,blockRotate,'/');
}

void DrawBlockWithFeatures(int y, int x, int blockID, int blockRotate){
    DrawRecommend(recommendY, recommendX, blockID, recommendR);
    DrawShadow(y,x,blockID,blockRotate);
    DrawBlock(y,x,blockID,blockRotate,' ');
}

void createRankList(){
	// user code
    int i;
    Node *new, *ptr=rankHead;
    FILE *fp=fopen("rank.txt", "r");
    
    if ( fp == NULL ) {
        rankHead=NULL;
        rankNum=0;
        return;
    }
    
    if(fscanf(fp, "%d", &rankNum)<1){
        rankHead=NULL;
        rankNum=0;
        return ;
    }
    
    for(i=0; i<rankNum; i++){
        new = (Node *)malloc(sizeof(Node));
        fscanf(fp, "%s", &new->name);
        fscanf(fp, "%d", &new->score);
        new->link=NULL;
        if(i==0)
            rankHead=ptr=new;
        else {
            ptr->link=new;
            ptr=ptr->link;
        }
    }
    fclose(fp);
}

void rank(){
	// user code
    int x, y, cnt, x_flag, y_flag;
    char in_name[NAMELEN];
    Node *ptr, *pre;
    clear();
    printw("1. list ranks from X to Y\n");
    printw("2. list ranks by a specific name\n");
    printw("3. delete a specific rank\n");
    
    switch(wgetch(stdscr)){
        case '1':
            echo();
            printw("X: ");
            x_flag=scanw("%d", &x);
            printw("Y: ");
            y_flag=scanw("%d", &y);
            printw("       name       |   score\n");
            printw("------------------------------\n");
            noecho();
            
            if(x_flag<1) x=1;
            if(y_flag<1) y=rankNum;
            
            if(y>rankNum) y=rankNum;
            
            if(x<=y && 0<x && y<=rankNum ){
                ptr=rankHead;
                for(cnt = 1; cnt<=rankNum; cnt++){
                    if(cnt>=x && cnt<=y)
                        printw(" %-16s | %d\n", ptr->name, ptr->score);
                    ptr=ptr->link;
                }
            }
            else{
                printw("search failure: no rank in the list\n");
            }
            break;
        case '2':
            echo();
            printw("input the name: ");
            scanw("%s", in_name);
            printw("       name       |   score\n");
            printw("------------------------------\n");
            noecho();
            
            x_flag=0;
            ptr=rankHead;
            for(cnt=1; cnt<=rankNum; cnt++){
                if(!strcmp(in_name, ptr->name)){
                    x_flag=1;
                    printw(" %-16s | %d\n", ptr->name, ptr->score);
                }
                ptr=ptr->link;
            }
            if(!x_flag)
                printw("search failure: no name in the list\n");
            break;
        case '3':
            echo();
            printw("input the rank: ");
            scanw("%d", &x);
            noecho();
            
            if(x<=0 || x>rankNum)
                printw("search failure: no rank in the list\n");
            else if(x==1){
                ptr=rankHead;
                rankHead=rankHead->link;
                free(ptr);
                printw("result : the rank deleted\n");
                rankNum--;
                writeRankFile();
            }
            else{
                ptr=rankHead->link;
                pre=rankHead;
                for(cnt=2; cnt<x; cnt++){
                    pre=ptr;
                    ptr=ptr->link;
                }
                pre->link=ptr->link;
                free(ptr);
                printw("result : the rank deleted\n");
                rankNum--;
                writeRankFile();
            }
            
            break;
        default:
            break;
    }
    getch();
}

void writeRankFile(){
	// user code
    FILE *fp=fopen("rank.txt", "w");
    
    fprintf(fp,"%d\n",rankNum);
    
    int i;
    Node *ptr=rankHead;
    for(i=0;i<rankNum;i++)
    {
        fprintf(fp,"%s %d\n",ptr->name,ptr->score);
        ptr=ptr->link;
    }
    fclose(fp);
}

void newRank(int score){
	// user code
    clear();
    
    Node *new = (Node *)malloc(sizeof(Node));
    new->score = score;
    
    printw("Name? ");
    echo();
    scanw("%s", new->name);
    noecho();
    new->link=NULL;
    
    Node *tmp=rankHead, *pre;
    
    if(!tmp) rankHead = new;
    
    else if(tmp->score < score ){
        new->link = tmp;
        rankHead = new;
    }
    else{
        while(1){
            pre=tmp;
            tmp=tmp->link;
            if(!tmp) break;
            if(tmp->score < score ) break;
        }
        pre->link = new;
        new->link = tmp;
    }
    rankNum++;
    writeRankFile();
    getch();
}

void DrawRecommend(int y, int x, int blockID,int blockRotate){
	// user code
    DrawBlock(y,x,blockID,blockRotate,'R');
}

int recommend(RecNode *root){
	int max=0; // 미리 보이는 블럭의 추천 배치까지 고려했을 때 얻을 수 있는 최대 점수
    
    int i, j, cnt=0;
    int cur_lv =  root->lv + 1;
    int cur_b = nextBlock[cur_lv];
    int rot;
    int most_r, most_l, r_bound, l_bound;
    int r, x, y;
    int score;
    int recX, recY, recR;
    RecNode *child;
    
    
    if( cur_b == 4) rot = 1;  // possible rotations of each blockID
    else if( cur_b == 0 || cur_b == 5 || cur_b == 6) rot = 2;
    else rot = 4;
    
    // get the x boundary of the block in the field
    for(r = 0 ; r < rot ; r++){
        most_r = -1;
        most_l = 4;
        for(i=0; i<BLOCK_HEIGHT; i++)
            for(j=0;j<BLOCK_WIDTH;j++){
                if(block[cur_b][r][i][j] == 1){
                    if(most_r<j) most_r=j;
                    if(most_l>j) most_l=j;
                }
            }
        l_bound = -most_l;
        r_bound = WIDTH-1-most_r;
        
        for(x=l_bound ; x<= r_bound ; x++){
            y=-1;
            while(CheckToMove(root->f,cur_b,r,y+1,x)){
                y++;
            }
            if(y==-1) continue;
            
            child = (RecNode*)malloc(sizeof(RecNode));
            child->f=(char(*)[WIDTH])malloc(sizeof(char)*HEIGHT*WIDTH);
            child->lv = cur_lv;
            
            for(i=0;i<HEIGHT;i++) // copy field
                for(j=0;j<WIDTH;j++)
                    child->f[i][j]=root->f[i][j];
            
            //calculate accumulated score
            score = root -> score;
            score += AddBlockToField(child->f, cur_b, r, y, x);
            score += DeleteLine(child->f);
            
            child->score = score;
            root->c[cnt++] = child;
            num_of_nodes++;
            
            if(cur_lv < VISIBLE_BLOCKS-1) score=recommend(child);
            
            if(score > max) { // update score and X, Y, R
                max = score;
                recX=x; recY=y; recR=r;
            }
            free(child->f);
            free(child);
        }
        
        if(cur_lv == 0){
            recommendR = recR;
            recommendY = recY;
            recommendX = recX;
        }
    
    }
	// user code
	return max;
}

void recommendedPlay(){
	int command;
    time_t start, stop;
    double duration;
    
    start = time(NULL);
    clear();
    act.sa_handler = recBlockDown;
    sigaction(SIGALRM,&act,&oact);
    InitTetris();
    do{
        if(timed_out==0){
            alarm(1);
            timed_out=1;
        }

        command = GetCommand();
        if(command=='q'||command=='Q'){
            alarm(0);
            DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
            move(HEIGHT/2,WIDTH/2-4);
            printw("Good-bye!!");
            refresh();
            getch();

            return;
        }
    }while(!gameOver);

    alarm(0);
    getch();
    DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
    move(HEIGHT/2,WIDTH/2-4);
    printw("GameOver!!\n");
    refresh();
    getch();
    clear();
    stop = time(NULL);
    duration=(double)difftime(stop, start);
    printw("score: %d, time: %f, number of nodes: %d\n", score, duration, num_of_nodes);
    printw("time efficiency: %f, memory efficiency: %f", score/duration, (float)score/num_of_nodes);
    getch();
}


void recBlockDown(int sig){
    int i, j;
    
    if(CheckToMove(field, nextBlock[0], blockRotate, blockY+1, blockX)){
        int shadow_y=blockY;
        while(CheckToMove(field,nextBlock[0],blockRotate,shadow_y+1,blockX)){
            shadow_y++;
        }
        int i,j;
        for(i=0;i<4;i++)
            for(j=0;j<4;j++){
                if(block[nextBlock[0]][blockRotate][i][j]==1 && i+blockY>=0){
                    move(i+blockY+1,j+blockX+1);
                    printw(".");
                    move(i+shadow_y+1,j+blockX+1);
                    printw(".");
                }
            }
        move(HEIGHT,WIDTH+10);
        blockY = recommendY;
        blockX = recommendX;
        blockRotate = recommendR;
        DrawBlockWithFeatures(blockY, blockX, nextBlock[0], blockRotate);
    }
    else {
        if(blockY == -1) gameOver = 1;
        score+=AddBlockToField(field, nextBlock[0], blockRotate, blockY, blockX);
        score+=DeleteLine(field);
        PrintScore(score);
        
        for(i=0; i<VISIBLE_BLOCKS-1; i++)
            nextBlock[i]=nextBlock[i+1];
        nextBlock[i]=rand()%7;
        
        blockRotate=0;
        blockY=-1;
        blockX=WIDTH/2-2;
        
        recRoot = (RecNode*)malloc(sizeof(RecNode));
        recRoot->f=(char(*)[WIDTH])malloc(sizeof(char)*HEIGHT*WIDTH);
        recRoot->lv = -1;
        recRoot->score = score;
        
        
        for(i=0;i<HEIGHT;i++) // copy field
            for(j=0;j<WIDTH;j++)
                recRoot->f[i][j]=field[i][j];
        //recommend(recRoot);    
        modified_recommend(recRoot);
        free(recRoot->f);
        free(recRoot);
        
        DrawNextBlock(nextBlock);
        DrawField();
        DrawBlockWithFeatures(blockY,blockX,nextBlock[0],blockRotate);
    }
    timed_out=0;
}

int modified_recommend(RecNode *root){
    int max=0; // 미리 보이는 블럭의 추천 배치까지 고려했을 때 얻을 수 있는 최대 점수
    
    int i, j, cnt=0;
    int cur_lv =  root->lv + 1;
    int cur_b = nextBlock[cur_lv];
    int rot;
    int most_r, most_l, r_bound, l_bound;
    int r, x, y;
    int score, score1, score2;
    int recX, recY, recR;
    int flag=0, p_score=0, p_x, p_y, p_r;
    RecNode *child;
    
    
    if( cur_b == 4) rot = 1;  // possible rotations of each blockID
    else if( cur_b == 0 || cur_b == 5 || cur_b == 6) rot = 2;
    else rot = 4;
    
    // get the x boundary of the block in the field
    for(r = 0 ; r < rot ; r++){
        most_r = -1;
        most_l = 4;
        for(i=0; i<BLOCK_HEIGHT; i++)
            for(j=0;j<BLOCK_WIDTH;j++){
                if(block[cur_b][r][i][j] == 1){
                    if(most_r<j) most_r=j;
                    if(most_l>j) most_l=j;
                }
            }
        l_bound = -most_l;
        r_bound = WIDTH-1-most_r;
        
        for(x=l_bound ; x<= r_bound ; x++){
            y=-1;
            while(CheckToMove(root->f,cur_b,r,y+1,x)){
                y++;
            }
            if(y==-1) continue;
            
            child = (RecNode*)malloc(sizeof(RecNode));
            child->f=(char(*)[WIDTH])malloc(sizeof(char)*HEIGHT*WIDTH);
            child->lv = cur_lv;
            
            for(i=0;i<HEIGHT;i++) // copy field
                for(j=0;j<WIDTH;j++)
                    child->f[i][j]=root->f[i][j];
            
            //calculate accumulated score
            score = root -> score;
            score1 = AddBlockToField(child->f, cur_b, r, y, x);
            score2 = DeleteLine(child->f);
            
            if(score2!=0 || score1>=(most_r-most_l+2)/2*10){
                num_of_nodes++;
                flag=1;
                score += score1+score2;
                child->score = score;
                root->c[cnt++] = child;
            
                if(cur_lv < VISIBLE_BLOCKS-1) score=recommend(child);
                if(score > max) { // update score and X, Y, R
                    max = score;
                    recX=x; recY=y; recR=r;
                }
            }
            
            else if(p_score < score1+score2){
                p_r=r;
                p_y=y;
                p_x=x;
                p_score=score1+score2;
            }
            
            free(child->f);
            free(child);
        }
        
        if(cur_lv == 0 && flag == 1 ){
            recommendR = recR;
            recommendY = recY;
            recommendX = recX;
        }
        //모든 branch가 pruning된 경우
        else if(cur_lv == 0 && flag == 0){
            recommendR = p_r;
            recommendY = p_y;
            recommendX = p_x;
        }
    
    }
    // user code
    return max;
}
