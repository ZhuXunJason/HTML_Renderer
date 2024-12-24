#include <ctype.h>
#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#define COL 51

/*思路:
 *
 *读取：初始化一个10*50,默认属性的div块，每次读入一个元素，将元素类型，属性，内容，位置，宽高等信息存入链表node中(包括div)
 *如果div没有初始宽、高信息，则同时更新所在div的宽高，元素个数等信息。
 *如果读入了div元素，则在录入父div块的同时新建一个div块，并进入新div块中读取元素，直到读入</div>，退出当前div块并返回至父div块。
 *
 *布局对齐：每次退出当前div块时执行对齐操作(即修改pos)，若有子div块，则利用递归对齐子div块的元素。
 *
 *输出：根据元素的pos输出至result中，如果元素为p或h，在StyleMem中记录样式，最后一并输出。
 */

/*
 *Element 记录 元素名(p, h, img,div)元素属性和元素内容
 *元素属性(Element.ele_attr)
 *  [0] 红色  [1] 蓝色   [2] 绿色   [3] 粗体  [4] 斜体  [5] 下划线
 *
 *  Division 记录 Element继承属性 和 div属性 和 div输出至result的开始位置
 *  div属性(Division.div_attr)
 *  [0]div布局 [1]div竖直居中对齐 [2]div竖直下对齐  [3]div竖直分散对齐 [4][5][6]div水平对齐方式
 *   [7]宽度是否为空 [8]高度是否为空
 */

typedef struct Element {
    int w, h;
    char *pos, type, str[501]; //存储元素输出至result的开始位置，元素类型，元素内容
    int ele_attr[6]; //如果元素为p或h，存储元素属性
    struct Division *div; //如果元素为div，指向该div
} Element;

typedef struct Division {
    int *w, *h; //用指针存储(指向该div在父div中相应的ele的w和h)，保证ele的w和h实时更新;
    char *DivPos;
    int div_attr[9];
    Element FatherEle; //子元素p和h的继承属性
    struct Division *FatherDiv;
    struct Node *node; //元素链表
    struct Node *node_as_ele; //div在父div中的节点
    int count; //division中的元素个数
} Division;

typedef struct Node {
    Element ele;
    struct Node *next;
} Node;

char result[10][COL] = {0};
char *StyleMem[10][COL] = {NULL}; //存储result对应位置的样式
const char *ele_style[] = {"color=\"red\"", "color=\"blue\"", "color=\"green\"", " em", " i", " u"};
const char *div_style[] = {
    "direction=\"column\"", "align-items=\"center\"", "align-items=\"end\"",
    "align-items=\"space-evenly\"", "justify-content=\"center\"", "justify-content=\"end\"",
    "justify-content=\"space-evenly\""
};

int w = 50, h = 10;
Division init = {&w, &h}; //初始化一个10*50,默认属性的div块
Division *cur_div = &init;
Node *cur_node;
char input[1000] = {0};
char *pos = &result[0][0]; //记录当前元素输出至result的开始位置

void output(Division *div);

void align(Division *div);

void scan(void);

void deviate(Division *div, int shift); //将div中的所有元素的位置全部偏移shift个单位

void free_pointers(Division *div);

int main(void) {
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < COL; j++)
            result[i][j] = 32; //将result的元素全部设为空格

    init.node = calloc(1, sizeof(Node)); //初始化链表的第一个节点
    cur_node = init.node;
    while (fgets(input, 999, stdin)) {
        //fgets()读取元素直至\n或EOF
        scan();
    }

    output(&init);
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < COL; j++) {
            if (StyleMem[i][j])
                printf("%s", StyleMem[i][j]); //如果对应位置有样式，则输出样式
            printf("%c", result[i][j] ? result[i][j] : 32);
        }
        printf("\n");
    }

    free_pointers(&init);
}

void scan(void) {
    if (strstr(input, "<p") || strstr(input, "<h") || strstr(input, "<img") || strstr(input, "<div")) {
        cur_div->count++; //当前div中的元素个数+1
        Node *new_node = calloc(1, sizeof(Node));
        cur_node->next = new_node;
        cur_node = new_node; //建立新节点，并将cur_node指向新节点
        Element *ele = &cur_node->ele; //ele储存当前元素
        ele->pos = pos;

        //p或h
        if (strstr(input, "<p") || strstr(input, "<h")) {
            while (!strstr(input, "</p>") && !strstr(input, "</h>"))
                fgets(input + strlen(input), 999, stdin);

            if (strstr(input, "<p"))
                ele->type = 'p';
            else
                ele->type = 'h';

            char *attr = strtok(input, ">");
            strcpy(ele->str, strtok(NULL, "<")); //strtok函数根据第二个参数分割字符串，第一个参数为NULL时，表示继续分割上一次的字符串
            for (int i = 0; i < 6; i++)
                if (strstr(attr, ele_style[i]))
                    ele->ele_attr[i] = 1;

            int len = strlen(ele->str);
            //大写标题h
            if (ele->type == 'h') {
                for (int i = 0; i < len; i++)
                    ele->str[i] = toupper(ele->str[i]);
            }

            ele->w = len;
            ele->h = 1;

            pos += cur_div->div_attr[0] ? ele->w : COL;

            //如果div的宽度或高度为空，则更新div的宽度或高度
            if (!cur_div->div_attr[7] && len > *cur_div->w)
                *cur_div->w = len;
            if (!cur_div->div_attr[8])
                *cur_div->h = cur_div->div_attr[0] ? 1 : *cur_div->h + 1;
        }

        //元素为img
        else if (strstr(input, "<img")) {
            while (!strstr(input, "</img>"))
                fgets(input + strlen(input), 999, stdin);

            ele->type = 'i';
            int width;
            char *a = strstr(input, "src=");
            char *b = strstr(input, "width=");
            sscanf(a, "src=\"%[^\"]\"", ele->str);
            sscanf(b, "width=\"%d\"", &width);

            ele->w = width;
            ele->h = (strlen(ele->str) + width - 1) / width;
            pos += cur_div->div_attr[0] ? ele->w : (strlen(ele->str) + width - 1) / width * COL;
        }

        //div元素
        else {
            while (!strchr(input, '>'))
                fgets(input + strlen(input), 999, stdin);

            ele->type = 'd';
            Division *new_div = calloc(1, sizeof(Division));

            //参见47行-56行division结构体的定义
            ele->div = new_div;
            new_div->FatherDiv = cur_div;
            new_div->FatherEle = cur_div->FatherEle;
            cur_div = new_div;

            cur_div->w = &ele->w;
            cur_div->h = &ele->h;
            cur_div->DivPos = ele->pos;
            cur_div->node_as_ele = cur_node;
            cur_div->node = calloc(1, sizeof(Node));
            cur_node = cur_div->node;

            // 读入元素属性和div属性
            char *attr = input;
            for (int i = 0; i < 6; i++)
                if (strstr(attr, ele_style[i]))
                    cur_div->FatherEle.ele_attr[i] = 1;
            for (int i = 0; i < 7; i++)
                if (strstr(attr, div_style[i]))
                    cur_div->div_attr[i] = 1;

            //读取div的 w 和 h
            char *a = NULL;
            if (a = strstr(attr, "w=")) {
                cur_div->div_attr[7] = 1;
                sscanf(a, "w=\"%d\"", cur_div->w);
            }
            if (a = strstr(attr, "h=")) {
                cur_div->div_attr[8] = 1;
                sscanf(a, "h=\"%d\"", cur_div->h);
            }
        }
    }

    if (strstr(input, "</div>")) {
        if (cur_div->FatherDiv->div_attr[0] == 0) //row布局
            pos = cur_div->DivPos + COL * *(cur_div->h);
        else //column布局
            pos = cur_div->DivPos + *(cur_div->w);

        align(cur_div);

        cur_node = cur_div->node_as_ele; //cur_node退回至上一div的链表中
        cur_div = cur_div->FatherDiv; //cur_div退回至上一div中
    }
}

void output(Division *div) {
    for (Node *i = div->node->next; i; i = i->next) {
        char *pos = i->ele.pos;
        int width = i->ele.w;

        if (i->ele.type == 'i') {
            int count = 0;
            while (i->ele.str[count]) {
                for (char *j = pos; j < pos + width && i->ele.str[count]; j++) {
                    sprintf(j, "%c", i->ele.str[count++]);
                }
                pos += COL; //pos换到下一行
            }
        }
        else if (i->ele.type == 'p' || i->ele.type == 'h') {
            sprintf(pos, "%s", i->ele.str);
            char **p = &StyleMem[0][0] + (pos - &result[0][0]); //**p在StyleMem中记录result对应位置样式

            char *attr[] = {"\033[31m", "\033[34m", "\033[32m", "\033[1m", "\033[3m", "\033[4m"};
            for (int j = 0; j < 6; j++) {
                if (i->ele.ele_attr[j] == 1) {
                    if (!(*p))
                        *p = calloc(25, sizeof(char));
                    strcat(*p, attr[j]);
                }
            }

            //样式重置
            if (*p && strcmp(*p, "\033[0m")) {
                char **end = p + strlen(i->ele.str);
                *end = calloc(25, sizeof(char));
                strcat(*end, "\033[0m");
            }
        }

        //遇到div则进行递归
        else if (i->ele.type == 'd') {
            output(i->ele.div);
        }
    }
}

void align(Division *div) {
    if (!div || !div->node)return;

    //div元素所有子元素的宽度之和和高度之和
    int wsum = 0, hsum = 0;
    for (Node *i = div->node->next; i; i = i->next) {
        wsum += i->ele.w;
        hsum += i->ele.h;
    }

    //根据div的布局方式和对齐属性确定偏移量x
    for (Node *i = div->node->next; i; i = i->next) {
        int x = 0;
        if (div->div_attr[1]) {
            x += div->div_attr[0] ? COL * ((*div->h - i->ele.h) / 2) : COL * ((*div->h - hsum) / 2);
        }
        else if (div->div_attr[2]) {
            x += div->div_attr[0] ? COL * (*div->h - i->ele.h) : COL * (*div->h - hsum);
        }
        else if (div->div_attr[3]) {
            static int count1 = 1;
            x += div->div_attr[0]
                     ? COL * ((*div->h - i->ele.h) / 2)
                     : COL * ((*div->h - hsum) / (div->count + 1)) * (count1++);
        }

        if (div->div_attr[4])
            x += div->div_attr[0] ? (*div->w - wsum) / 2 : (*div->w - i->ele.w) / 2;
        else if (div->div_attr[5])
            x += div->div_attr[0] ? (*div->w - wsum) : (*div->w - i->ele.w);
        else if (div->div_attr[6]) {
            static int count2 = 1;
            x += div->div_attr[0]
                     ? (*div->w - wsum) / (div->count + 1) * (count2++)
                     : (*div->w - i->ele.w) / 2;
        }
        i->ele.pos += x;
        if (i->ele.type == 'd')
            deviate(i->ele.div, x);
    }
}

void deviate(Division *div, int shift) {
    if (!div || !div->node || !shift)return;
    for (Node *i = div->node->next; i; i = i->next) {
        i->ele.pos += shift;
        if (i->ele.type == 'd')
            deviate(i->ele.div, shift);
    }
}

void free_pointers(Division *div) {
    if (!div || !div->node)return;
    for (Node *i = div->node->next; i;) {
        if (i->ele.type == 'd')
            free_pointers(i->ele.div);
        Node *tmp = i;
        i = i->next;
        free(tmp);
    }
    free(div->node);
    if (div != &init)
        free(div);
}
