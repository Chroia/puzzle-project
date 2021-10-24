#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* 文字列関数を扱えるようにする */
#include <time.h>

#define W 10   /* W = 最大長さ，ここでは 10 に設定 */
#define n 3001 /* n = リストで扱う頂点の最大数，ここでは 3001 に設定 */
#define NIL -1
#define maxN 1000 //キューに入れる頂点数

struct cell
{ /* 構造体 cell の定義 */
    int key;
    int dist;               //始点からの距離
    int parent;             //親
    unsigned int state : 2; //0:Empty, 1:Occupied, 2:Deleted
};

int A[maxN];    /* キューを表す変数 */
int head, tail; /* キューの先頭と末尾 */

int expo(int k, int p);
int change_radix(int k, int p, int x);
void hash_print(struct cell *B, int a);

//構造B(内部ハッシュ法)
int hash_func(int v);
int hash_func_cnt(int h);
int hash_search(struct cell *B, int a);
void hash_insert(struct cell *B, int a, int v, int vdist);

//線形リスト用(未使用セルの取得)
int allocate_object(struct cell *L, int *freeL);

//Horner法(隣接頂点の桁表現に対応するd進数を計算)
int Horner_computation(int *vert, int d, int l);

//キューA
void enqueue(int a);
int dequeue(void);
void BFS(int v0);

void move(int a, int b, int c, int *vert, int *vert1);

int main(void)
{
    struct cell B[n]; /* Bは ハッシュ表を表す cell の配列，数値数は n まで */
    int i, j, k, h /*, l*/;
    int /*x,*/ y, u, v;
    int v0;         //v0 = 最初の頂点
    int config[16]; //盤面の配列
    int config1[16];
        int count[16]={0}; //最小値を保存する
    int vdist; //vのdistを保存する

    clock_t start_t, end_t;
    double utime;

    //時間計測開始
    start_t = clock();

    for (j = 0; j < 3; j++)
    {

        for (h = 0; h < n; h++)
        {
            B[h].state = 0;
        } /* ハッシュ表の初期化 */

        //初期値
        for (i = 0; i < 16; i++)
        {
            config[i] = 1;
        }
        if (j == 2)
        {
            config[5] = 0;
        }
        else
        {
            config[j] = 0;
        }

        v0 = Horner_computation(config, 2, 16); //10進数に直す

        // BFS ----------------------------------------------
        head = tail = 0;

        enqueue(v0);

        //キューAが空になるまで続ける
        while (head != tail)
        {
            v = dequeue(); //頂点vを取り出してくる
            vdist = B[hash_search(B, v)].dist;

            //vを2進表記の配列に直す
            k = v;
            for (i = 15; i >= 0; i--)
            {
                config[i] = k % 2;
                k = k / 2;
            }

            //子の計算
            for (i = 0; i < 16; i++)
            {
                if (config[i] == 0)
                {

                    if (i / 8 == 0)
                    { //上半分で(8未満であれば0になることを利用)
                        if (config[i + 4] == 1 && config[i + 8] == 1) //下を見る
                        {
                            move(i, i + 4, i + 8, config, config1);
                        }
                    }
                    else// if(i/8 == 1)　//下半分で
                    {
                        if (config[i - 4] == 1 && config[i - 8] == 1) //上を見る
                        {
                            move(i, i - 4, i - 8, config, config1);
                        }
                    }
                    if ((i / 2) % 2 == 0)
                    { //左半分で(2で割った商が偶数になることを利用)
                        if (config[i + 1] == 1 && config[i + 2] == 1)//右を見る
                        {
                            move(i, i + 1, i + 2, config, config1);
                        }
                    }
                    else// if((i / 2) % 2 == 1) //右半分で
                    {
                        if (config[i - 1] == 1 && config[i - 2] == 1)//左を見る
                        {
                            move(i, i - 1, i - 2, config, config1);
                        }
                    }
                    
                    

                    u = Horner_computation(config1, 2, 16); //10進数に直す
                    y = hash_search(B, u);
                    if (y == NIL)
                    {
                        hash_insert(B, u, v, vdist); //自分と親を渡す
                        enqueue(u);
                    }
                }
            }

            if(j==2)
            {
                count[5]=16-B[hash_search(B, u)].dist;
            }
            else
            {
                count[j]=16-B[hash_search(B, u)].dist;
            }
        
        }
        // BFSおわり----------------------------------------

    }
    
    count[15]=count[0];
    count[12]=count[0];
    count[3]=count[0];
    count[10]=count[5];
    count[9]=count[5];
    count[6]=count[5];
    count[14]=count[1];
    count[13]=count[1];
    count[11]=count[1];
    count[8]=count[1];
    count[7]=count[1];
    count[4]=count[1];
    count[2]=count[1];
    
    end_t = clock();
    utime = (double)(end_t - start_t) / CLOCKS_PER_SEC; //開始と終了の差を取り秒に換算
    printf("time = %lf\n", utime);

    for(i=0;i<16;i++)
    {
        printf("initial(%d):min=%d\n", i, count[i]);
    }

    return 0;
}

//長さlの配列vertに格納された桁表現から対応するd進数を計算する
int Horner_computation(int *vert, int d, int l)
{
    int i, v;
    v = vert[0];

    for (i = 1; i < l; i++)
    {
        v = d * v + vert[i];
    }
    return v;
}

//キューAの末尾にuを挿入する
void enqueue(int u)
{
    A[tail] = u;
    tail += 1;

    if (tail >= maxN)
    { //tailの値が最後尾を超えたら0(アドレス的な意味の先頭)に移す
        tail = 0;
    }
    if (tail == head)
    { //tailとheadが同じつまりAが満杯になったらエラー出力
        printf("overflow\n");
        exit(1);
    }
}

//キューAの先頭の要素を返す
int dequeue(void)
{
    int v;

    if (head == tail)
    { //headとtailが同じつまりAが空ならエラー出力
        printf("underflow\n");
        exit(1);
    }
    else
    {
        v = A[head];
        head += 1;
        if (head >= maxN)
        { //headの値が最後尾を超えたら0に移す
            head = 0;
        }
        return v;
    }
}

//ハッシュ関数で計算された結果を返す
int hash_func(int v)
{
    //渡す値は最高でも65535よりオーバーフローは起きない
    int h;
    //int dig; //各桁の数字
    h = 0;
    /*while (v)
    { //各桁の数字の和を計算 //内部ハッシュだとバケット多いから非効率的
        dig = v % 10;
        h = h + dig;
        v = v / 10;
    }*/
    h = h % n; /* m で割った余りを取る */
    return h;
}

//2重ハッシュ法の二つ目のハッシュ関数
int hash_g(int h)
{
    h = 911 - (h % 911);
    return h;
}

//2度目以降のハッシュ関数
int hash_func_cnt(int h)
{
    h = h % n;
    return h;
}

//ハッシュ表Bにaがあるか探索
int hash_search(struct cell *B, int a)
{
    int h, g, x, z, i;
    int end = 0;
    h = hash_func(a);
    g = hash_g(a);
    x = NIL;
    i = 0;
    while (end == 0)
    {
        z = B[hash_func_cnt(h + i * g)].state;
        if (z == 1 && B[hash_func_cnt(h + i * g)].key == a)
        { //一致しててOccupiedだったら
            x = hash_func_cnt(h + i * g);
            end = 1;
        }
        else
        {
            if (z == 0)
            { //EmptyよりxはNILのまま返される
                end = 1;
            }
            else
            {
                i++;
                if (i >= n)
                {
                    printf("error:out of space\n");
                    end = 1;
                }
            }
        }
    }
    return x;
}

//ハッシュ表Bにないkeyを挿入する
void hash_insert(struct cell *B, int a, int v, int vdist)
{
    int h, g, x, i;
    h = hash_func(a);
    g = hash_g(a);
    x = NIL;
    i = 0;
    while (x < 0 && i < n)
    {
        if (B[hash_func_cnt(h + i * g)].state != 1)
        {
            x = hash_func_cnt(h + i * g);
        }
        else
        {
            i++;
        }
    }

    if (x < 0)
    {
        printf("error: out of space");
    }
    else
    {
        B[x].key = a;
        B[x].dist = vdist + 1;
        B[x].parent = v; //親は直前にAからとってきた頂点
        B[x].state = 1;  //Occupied
    }
}

void move(int a, int b, int c, int *vert, int *vert1)
{
    int i;
    for (i = 0; i < 16; i++)
    {
        vert1[i] = vert[i];
    }
    vert1[a] = 1;
    vert1[b] = 0;
    vert1[c] = 0;
}

//key=aの盤面までの過程を出力する

void hash_print(struct cell *B, int a)
{
    int x, i, j, k;
    int Data[16] = {NIL}, vert[16];
    x = a;
    k = 0;
    for (i = 0; i < 16; i++)
    {
        if (B[x].key != NIL && x != NIL)
        {
            Data[i] = B[x].key;
            x = hash_search(B, B[x].parent);
        }
    }

    for (j = 15; j > -1; j--)
    {
        for (i = 0; i < 16; i++)
        {
            if (Data[i] != NIL)
            {
                vert[i] = change_radix(2, 15 - i, Data[j]);
            }
        }
        if (Horner_computation(vert, 2, 16) != 0)
        {
            printf("time=%d:\n", k);
            k++;
        }

        for (i = 0; i < 16; i++)
        {
            if (Horner_computation(vert, 2, 16) != 0)
            {
                printf("%d ", vert[i]);
                if (i == 3 || i == 7 || i == 11 || i == 15)
                {
                    printf("\n");
                }
            }
        }
    }
}

//xをk進法に変換し、k^pの係数を返す
int expo(int k, int p)
{
    int i, e = k;
    if (p > 0)
    {
        for (i = 1; i < p; i++)
        {
            e = e * k;
        }
    }
    else if (p == 0)
    {
        e = 1;
    }

    return e;
}
int change_radix(int k, int p, int x)
{
    int a, b;
    if (expo(k, p) <= x)
    {
        a = x / expo(k, p);
        b = a % k;
    }
    else
    {
        b = 0;
    }

    return b;
}
