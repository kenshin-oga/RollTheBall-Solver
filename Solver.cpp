#include "DxLib.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <windows.h>


#define PuzzleSize 16//パズルの大きさ
#define id 1
#define idastar1 2
#define idastar2 3
#define stage_sum 7

 

// グローバル変数
unsigned int Color = GetColor(255, 255, 255);
unsigned int Color2 = GetColor(255, 0, 255);
char filename[500];
enum st { METHOD, STAGE, START, MAIN, OPTION };
clock_t start_clock, end_clock;
int x1 = 0, y1 = 0, x2, y2;
int pic_road[25];
int status = METHOD;
int VisitedNum = 0;
int dfscount = 0;
int dfsflg = 0;
int changecount = 0;
int changelog1[10000000] = {};
int changelog2[10000000] = {};
int roopcount = 0;
int count = 0;
int move_panel[100];
int move_space[100];
int method_num = 1;
int stage_num = 1;
bool KEYDOWNflg = 0;
bool animeflg = 0;


int Stage[20][16] = { { 22,21,21,21,     /*2手問題*/
                         22, 2, 2,16,
                          6, 2,18,22,
                          5,21,22,21 },

                       {22,22,22,22,     /*5手問題*/
                         2, 6,18, 3,
                         5, 2, 2,22,
                        14, 8, 2, 4 },

                       { 2,22, 1, 3,       /*7手問題*/
                        22,14, 8,22,
                         6, 2, 2, 4,
                         2, 5, 2,18 },

                       { 2, 2, 2,17,      /*9手問題*/
                        12, 4, 2,22,
                         5,22,22, 4,
                        14, 2, 2, 3 },

                       { 6, 1, 9,15,      /*17手問題*/
                        17, 1, 1, 7,
                         4, 5, 1,22,
                         5, 4, 1, 1 },

                       { 22, 1,22,22,      /*15手問題*/
                          2,15,22,22,
                          6, 1, 7, 3,
                          4, 5,19, 4 },

                       { 15, 6,22, 3,
                         22,22,19,21,
                          1, 7, 4,21,
                          1, 1, 5,21 } };

int Stage_clear[20][16] =    /*2手問題*/
                       { { 0, 0, 0, 0,
                           6, 2, 2,16,
                           5, 2,18, 0,
                           0, 0, 0, 0 }, //0は任意のパネル

                              /*5手問題*/
                       {  0, 0, 0, 0,
                          6, 2,18, 0,
                          5, 2, 2, 3,
                         14, 8, 2, 4 },
                             /*7手問題*/
                       { 0, 0, 0, 0,
                          0,14, 8, 3,
                          6, 2, 2, 4,
                          5, 2, 2,18 },
                            /*9手問題*/
                       { 0, 0, 0,17,
                        12, 2, 2, 4,
                         5, 3, 0, 0,
                        14, 4, 0, 0 },
                           /*17手問題*/
                       { 0, 6, 9,15,
                        17, 1, 1, 7,
                         1, 1, 1, 1,
                         5, 4, 5, 4 },
                           /*15手問題*/
                       {  6, 2, 3, 0,
                          1,15, 1, 0,
                          5, 4, 7, 0,
                          0, 0,19, 0 },

                       {15, 6, 3, 0,
                         1, 1,19, 0,
                         1, 7, 0, 0,
                         5, 4, 0, 0 } };

 
int stage[16] = {};
int stage_clear[16] = {};
int stage_[16] = {};
FILE* fp;

typedef struct {
    int dir1;
    int dir2;
} p_dir;

p_dir p_dir1[23] =
{
    {NULL,NULL},
    {-4,4},
    {1,-1},
    {4,-1},
    {-4,-1},
    {-4,1},
    {1,4},

    {-4,4},
    {1,-1},
    {4,-1},
    {-4,-1},
    {-4,1},
    {1,4},

    {4,NULL},
    {1,NULL},
    {-4,NULL},
    {-1,NULL},
    {-4,NULL},
    {-1,NULL},
    {4,NULL},
    {1,NULL},
    {NULL,NULL},
    {NULL,NULL},
};

void change_x_y(int p, int* x, int* y) {
    //パネル番号をx,yに直す
    *x = 0, * y = 0;
    while (1) {
        if (p - 4 < 0) {
            *x = p;
            break;
        }
        p -= 4;
        *y += 1;
    }
}

// 選択したパネル２つをを入れ替える
void change(int x1, int y1, int x2, int y2, int* stage__) {
    //ステージ配列の添え字に対応させる
    int p1 = y1 * 4 + x1;
    int p2 = y2 * 4 + x2;;
    int tmp;

    //パネルが隣り合っているかどうか
    if (p2 == p1 - 1 || p2 == p1 + 1 || p2 == p1 - 4 || p2 == p1 + 4) {
        tmp = stage__[p1];
        stage__[p1] = stage__[p2];
        stage__[p2] = tmp;
    }
}

//パネルが固定かどうか調べる(固定なら0を返す)
int CheckFixed(int x, int y, int* stage___) {
    int p = y * 4 + x;
    if (stage___[p] >= 7 && stage___[p] <= 20)return 0;
    else return 1;
}

//パネルが固定かどうか調べる(パネル番号が引数Ver.)
int CheckFixed2(int p, int* stage___) {
    if (stage___[p] >= 7 && stage___[p] <= 20)return 0;
    else return 1;
}

//パネルがない場所かどうか調べる(ないなら0を返す)
int CheckSpace(int x, int y, int* stage___) {
    int p = y * 4 + x;
    if (stage___[p] == 22)return 0;
    else return 1;
}

//パネルがない場所と隣接しているか調べる(隣接しているなら1を返す)
int WithSpace(int p, int x, int y, int* stage__) {
    if (y > 0 && stage__[p - 4] == 22)
        return 1;
    if (x < 3 && stage__[p + 1] == 22)
        return 1;
    if (y < 3 && stage__[p + 4] == 22)
        return 1;
    if (x > 0 && stage__[p - 1] == 22)
        return 1;
    return 0;
}

int CheckClear2() {
    // クリア判定
    for (int i = 0; i < PuzzleSize; i++) {
        if (stage[i] != stage_clear[i])
            if (stage_clear[i] != 0)return 0;
    }
    return 1;
}
int CheckClear() {
    // クリア判定
    int p = 0, x, y, pre_d, next_d;
    for (int i = 0; i < PuzzleSize; i++) {
        if (stage[i] >= 13 && stage[i] <= 16) {
            p = i;
            break;
        }
    }
    change_x_y(p, &x, &y);
    //スタートの先が場外ならだめ
    if (p_dir1[stage[p]].dir1 == -1 && x == 0) {
        return 0;
    }
    else if (p_dir1[stage[p]].dir1 == -4 && y == 0) {
        return 0;
    }
    else if (p_dir1[stage[p]].dir1 == 1 && x == 3) {
        return 0;
    }
    else if (p_dir1[stage[p]].dir1 == 4 && y == 3) {
        return 0;
    }

    //スタートの先のパネルとつながっているか
    if (p_dir1[stage[p + p_dir1[stage[p]].dir1]].dir1 != (-1) * p_dir1[stage[p]].dir1
        && p_dir1[stage[p + p_dir1[stage[p]].dir1]].dir2 != (-1) * p_dir1[stage[p]].dir1)
        return 0;

    p += p_dir1[stage[p]].dir1;
    pre_d = p_dir1[stage[p]].dir1;
    while (stage[p] < 17 || stage[p]>20) {
        if (p_dir1[stage[p]].dir1 == (-1) * pre_d)next_d = p_dir1[stage[p]].dir2;
        else next_d = p_dir1[stage[p]].dir1;

        //パネルの先が場外ならだめ
        if (next_d == -1 && x == 0) {
            return 0;
        }
        else if (next_d == -4 && y == 0) {
            return 0;
        }
        else if (next_d == 1 && x == 3) {
            return 0;
        }
        else if (next_d == 4 && y == 3) {
            return 0;
        }
        if (p_dir1[stage[p + next_d]].dir1 != (-1) * next_d
            && p_dir1[stage[p + next_d]].dir2 != (-1) * next_d)
            return 0;

        p += next_d;
        pre_d = next_d;
    }
    return 1;
}

//配列コピー関数 arr1←arr2
void CopyArr(int* arr1, int* arr2) {
    for (int i = 0; i < 16; i++) {
        arr1[i] = arr2[i];
    }
}

//#if idastar2
//現ノードからゴールまでの推測値
int h_2() {
    int cost = 0,x1,y1,x2,y2;
    for (int i = 0; i < PuzzleSize; i++) {
        for (int j = 0; j < PuzzleSize; j++) {
            /*正解のルート上かつ固定じゃないパネルなら*/
            if (stage_clear[j] != 0 && CheckFixed2(j,stage_clear) == 1) {
                if (stage[i] == stage_clear[j]) {
                    change_x_y(i, &x1, &y1);
                    change_x_y(j, &x2, &y2);
                    cost += abs(x1 - x2) + abs(y1 - y2);
                    break;
                }
            }
        }
    }
    return cost;
}
//#endif

//#if idastar1
int h_1() {
    int cost = 0;
    for (int i = 0; i < PuzzleSize; i++) {
        if (stage[i] != 0 && stage[i] != stage_clear[i])
            cost++;
    }
    return cost;
}
//#endif

//#if idastar1 || idastar2

void dfs_idastar(int limit, int move) {
    int num = 0;
    //ノード数一定で書き込み
    dfscount++;
    if (dfscount % 10000000 == 0) {
        
        //書き込む
        fprintf(fp, "探索ノード数%d limit値%d\n", dfscount, move);
        
    }
    else {
        if (method_num == idastar1)num = move + h_1();
        else if (method_num == idastar2)num = move + h_2();
        if (num <= limit) {

            if (CheckClear2() == 1) {
                count += 1;
                
                //書き込む
                fprintf(fp, "\n手数:%d\n", move);
                int i = 1;
                while (move_panel[i] != -1){
                    fprintf(fp, "%d -> %d\n", move_panel[i], move_space[i]);
                    i++;
                }
                

                CopyArr(stage_, stage);
            }

            else {
                for (int i = 0; i < PuzzleSize; i++) {
                    //パズル内の空白パネルを探す
                    if (stage[i] == 22) {
                        //隣接4方向調べる
                        for (int j = 1; j <= 4; j++) {
                            int p, x;
                            if (j == 1) {
                                if (i >= 0 && i <= 3)continue;
                                x = i - 4;
                            }
                            else if (j == 2) {
                                if (i == 3 || i == 7 || i == 11 || i == 15)continue;
                                x = i + 1;
                            }
                            else if (j == 3) {
                                if (i >= 12 && i <= 15)continue;
                                x = i + 4;
                            }
                            else if (j == 4) {
                                if (i == 0 || i == 4 || i == 8 || i == 12)continue;
                                x = i - 1;
                            }

                            p = stage[x];
                            if (move_space[move] == x)continue;
                            if ((stage[x] >= 7 && stage[x] <= 20) || stage[x] == 22)continue;

                            //パネルを動かす
                            stage[i] = p;
                            stage[x] = 22;

                            if (animeflg == true) {
                                // パネルの描画
                                for (int i = 0; i <= 15; i++) {
                                    DrawGraph((i % 4) * 80, (i / 4) * 80, pic_road[stage[i]], FALSE);
                                }
                                DrawFormatString(20, 50, Color2, "探索中です……");
                                ScreenFlip();
                                //Sleep(20);
                            }

                            //手順記録
                            move_panel[move + 1] = x;
                            move_space[move + 1] = i;
                            //再起
                            dfs_idastar(limit, move + 1);
                            //元に戻す
                            stage[i] = 22;
                            stage[x] = p;
                        }
                    }
                }
            }
        }
    }
}

int dfsSolver_idastar() {
    count = 0;
    int i;
    if (method_num == idastar1)i = h_1();
    else if (method_num == idastar2)i = h_2();
    fp = fopen("movelog.txt", "w");
    for (i <= 300; i++;) {
        dfs_idastar(i, 0);
        if (count > 0) {
            fclose(fp);
            return 1;
        }
    }
    return 0;
}
//#endif

//#if id
void dfs_id(int limit, int move) {
    dfscount++;
    if (dfscount % 10000000 == 0) {
        
        //書き込む
        fprintf(fp, "探索ノード数%d limit値%d\n", dfscount, limit);
        
    }
    if (move == limit) {
        if (CheckClear2() == 1) {
            count += 1;
            
            //書き込む
            fprintf(fp, "\n手数:%d\n", limit);
            int i = 1;
            while (move_panel[i] != -1) {
                fprintf(fp, "%d -> %d\n", move_panel[i], move_space[i]);
                i++;
            }


            CopyArr(stage_, stage);
        }
    }
    else {
        for (int i = 0; i < PuzzleSize; i++) {
            //パズル内の空白パネルを探す
            if (stage[i] == 22) {
                //隣接4方向調べる
                for (int j = 1; j <= 4; j++) {
                    int p, x;
                    if (j == 1) {
                        if (i >= 0 && i <= 3)continue;
                        x = i - 4;
                    }
                    else if (j == 2) {
                        if (i == 3 || i == 7 || i == 11 || i == 15)continue;
                        x = i + 1;
                    }
                    else if (j == 3) {
                        if (i >= 12 && i <= 15)continue;
                        x = i + 4;
                    }
                    else if (j == 4) {
                        if (i == 0 || i == 4 || i == 8 || i == 12)continue;
                        x = i - 1;
                    }

                    p = stage[x];
                    if (move_space[move] == x)continue;
                    if ((stage[x] >= 7 && stage[x] <= 20) || stage[x] == 22)continue;

                    //パネルを動かす
                    stage[i] = p;
                    stage[x] = 22;

                    if (animeflg == true) {
                        // パネルの描画
                        for (int i = 0; i <= 15; i++) {
                            DrawGraph((i % 4) * 80, (i / 4) * 80, pic_road[stage[i]], FALSE);
                        }
                        DrawFormatString(20, 50, Color2, "探索中です……");
                        ScreenFlip();
                        //Sleep(20);
                    }

                    //手順記録
                    move_panel[move + 1] = x;
                    move_space[move + 1] = i;
                    //再起
                    dfs_id(limit, move + 1);
                    //元に戻す
                    stage[i] = 22;
                    stage[x] = p;
                }
            }
        }
    }

}


int dfsSolver_id() {
    count = 0;
    fp = fopen("movelog.txt", "w");
    for (int i = 1; i <= 30; i++) {
        dfs_id(i, 0);
        if (count > 0) {
            fclose(fp);
            return 1;
        }
    }
    return 0;
}
//#endif

// 探索するSolverを選択
void selectMETHOD() {
    
    if (CheckHitKey(KEY_INPUT_RETURN) == 1) {
        KEYDOWNflg = 0;
        status = STAGE;
        Sleep(200);
    }
    if (CheckHitKey(KEY_INPUT_Q) == 1) {
        KEYDOWNflg = 0;
        status = OPTION;
        Sleep(200);
    }
    // 文字の描画
    DrawString(80, 40, "使うSolverを選んでね", Color);
    DrawString(120, 110, "反復深化法", Color2);
    DrawString(120, 160, "IDASTAR 1(優秀)", Color2);
    DrawString(120, 210, "IDASTAR 2(ザコ)", Color2);
    DrawString(80, 60+method_num * 50, "⇒", Color);
    DrawString(20, 270, "q:探索設定", Color);

    if (CheckHitKey(KEY_INPUT_DOWN) == 1 && KEYDOWNflg==0)
    {
        KEYDOWNflg = 1;
        if (method_num == idastar2)method_num = id;
        else method_num++;
        Sleep(200);
        KEYDOWNflg = 0;
    }
    if (CheckHitKey(KEY_INPUT_UP) == 1 && KEYDOWNflg == 0)
    {
        KEYDOWNflg = 1;
        if (method_num == id)method_num = idastar2;
        else method_num--;
        Sleep(200);
        KEYDOWNflg = 0;
    }

}

void anime_OPTION() {

    if (CheckHitKey(KEY_INPUT_RETURN) == 1) {
        KEYDOWNflg = 0;
        status = METHOD;
        Sleep(200);
    }
    
    // 文字の描画
    DrawString(50, 40, "パズルのスライドを描画する", Color);
    DrawString(20, 70, "※する場合探索速度が格段に落ちます", Color);
    DrawString(140, 160, "する", Color2);
    DrawString(140, 210, "しない", Color2);
    DrawString(80, 210-animeflg*50, "⇒", Color);
    //DrawString(20, 270, "q:戻る", Color);

    if (CheckHitKey(KEY_INPUT_DOWN) == 1 && KEYDOWNflg == 0)
    {
        KEYDOWNflg = 1;
        if (animeflg == 1)animeflg = 0;
        else animeflg++;
        Sleep(200);
        KEYDOWNflg = 0;
    }
    if (CheckHitKey(KEY_INPUT_UP) == 1 && KEYDOWNflg == 0)
    {
        KEYDOWNflg = 1;
        if (animeflg == 1)animeflg = 0;
        else animeflg ++;
        Sleep(200);
        KEYDOWNflg = 0;
    }
}

//問題を選択
void selectSTAGE() {

    x1 = 0, y1 = 0;
    VisitedNum = 0;
    dfscount = 0;
    changecount = 0;
    changelog1[10000000] = {};
    changelog2[10000000] = {};
    roopcount = 0;



    if (CheckHitKey(KEY_INPUT_RETURN) == 1) {
        CopyArr(stage, Stage[stage_num - 1]);
        CopyArr(stage_clear, Stage_clear[stage_num - 1]);
        status = START;
        Sleep(200);
    }
    // 文字の描画
    DrawString(20, 270, "q:戻る", Color);
    DrawString(100, 20, "問題を選んでね", Color);
    DrawString(120, 60, "2手問題", Color2);
    DrawString(120, 90, "5手問題", Color2);
    DrawString(120, 120, "7手問題", Color2);
    DrawString(120, 150, "9手問題", Color2);
    DrawString(120, 180, "17手問題", Color2);
    DrawString(120, 210, "15手問題1(激むず)", Color2);
    DrawString(120, 240, "15手問題2", Color2);
    DrawString(80, 30+stage_num * 30, "⇒", Color);

    if (CheckHitKey(KEY_INPUT_Q) == 1)status = METHOD;

    if (CheckHitKey(KEY_INPUT_DOWN) == 1 && KEYDOWNflg == 0)
    {
        KEYDOWNflg = 1;
        if (stage_num == stage_sum)stage_num = 1;
        else stage_num++;
        Sleep(200);
        KEYDOWNflg = 0;
    }
    if (CheckHitKey(KEY_INPUT_UP) == 1 && KEYDOWNflg == 0)
    {
        KEYDOWNflg = 1;
        if (stage_num == 1)stage_num = stage_sum;
        else stage_num--;
        Sleep(200);
        KEYDOWNflg = 0;
    }

}

// 探索前の局面を表示(クリックで探索開始)
void gameSTART() {
    dfsflg = 0;
    if (CheckHitKey(KEY_INPUT_RETURN) == 1) {
        KEYDOWNflg = 0;
        status = MAIN;
        Sleep(200);
    }

    // パネルの描画
    for (int i = 0; i <= 15; i++) {
        DrawGraph((i % 4) * 80, (i / 4) * 80, pic_road[stage[i]], FALSE);
    }
    DrawFormatString(20, 20, Color2, "Enterで探索開始！");

}

// メイン画面
void gameMain() {
    if (dfsflg == 0) {
        // パネルの描画
        for (int i = 0; i <= 15; i++) {
            DrawGraph((i % 4) * 80, (i / 4) * 80, pic_road[stage[i]], FALSE);
        }
        DrawFormatString(20, 50, Color2, "探索中です……");
        ScreenFlip();
    }
    else {
        // パネルの描画
        for (int i = 0; i <= 15; i++) {
            DrawGraph((i % 4) * 80, (i / 4) * 80, pic_road[stage_[i]], FALSE);
        }
    }
    

    //探索は１度だけ
    if (dfscount == 0) {
        /* 処理開始前のクロックを取得 */
        start_clock = clock();
        
        switch (method_num) {
        case 1:
            if (dfsSolver_id() == 1)
                dfsflg = 1;
            break;
        case 2:
            if (dfsSolver_idastar() == 1)
                dfsflg = 1;
            break;
        case 3:
            if (dfsSolver_idastar() == 1)
                dfsflg = 1;
            break;

        }

        /* 処理終了後のクロックを取得 */
        end_clock = clock();
    }
    if (dfsflg == 1) {
        DrawFormatString(0, 0, Color2, "探索成功！");
        DrawFormatString(80, 0, Color2, "探索ノード総数：%d", dfscount);
        DrawFormatString(0, 30, Color2, "処理時間：%f", (double)(end_clock - start_clock) / CLOCKS_PER_SEC);
        DrawFormatString(0, 60, Color2, "正解手は%dパターンありました", count);
        DrawFormatString(0, 90, Color2, "手順はmovelog.txtをチェック！");
    }
    else DrawFormatString(0, 0, Color2, "探索失敗！");

    DrawString(20, 270, "q:探索法選択に戻る", Color2);
    DrawString(20, 290, "w:問題選択に戻る", Color2);
    if (CheckHitKey(KEY_INPUT_Q) == 1 && KEYDOWNflg == 0)
    {
        KEYDOWNflg = 1;
        status = METHOD;
        Sleep(200);
        KEYDOWNflg = 0;
    }
    if (CheckHitKey(KEY_INPUT_W) == 1)status = STAGE;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    SetGraphMode(320, 320, 32);
    ChangeWindowMode(TRUE);
    DxLib_Init();
    SetDrawScreen(DX_SCREEN_BACK);
    for (int i = 1; i <= 22; i++) {
        sprintf(filename, "road%d.png", i);
        pic_road[i] = LoadGraph(filename);
    }
    for (int i = 0; i < 100; i++) {
        move_panel[i] = -1;
        move_space[i] = -1;
    }
    
    while (!ProcessMessage()) {
        ClearDrawScreen();
        switch (status) {
        case METHOD: selectMETHOD(); break;
        case OPTION: anime_OPTION(); break;
        case STAGE: selectSTAGE(); break;
        case START: gameSTART(); break;
        case MAIN: gameMain(); break;
        }
        ScreenFlip();
    }
    DxLib_End();
    return 0;
}