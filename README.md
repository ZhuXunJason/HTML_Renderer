# 一个简易版HTML渲染器

## 题目信息

#### [题目链接:](https://git.nju.edu.cn/huorunz/browserinterminal)https://git.nju.edu.cn/huorunz/browserinterminal

#### 题目描述:

在终端中显示网页内容，网页内容由p,h,img,div四种元素组成。

**输入**：输入为一段HTML代码，其中包含p,h,img,div四种元素。

**输出**：一个10 * 50大小的字符渲染结果

## 思路:

#### 读取：

- 初始化一个10 * 50,默认属性的div块，每次读入一个元素，将元素类型，属性，内容，位置，宽高等信息存入链表node中(包括div)
- 如果div没有初始宽、高信息，则同时更新所在div的宽高，元素个数等信息。
- 如果读入了div元素，则在录入父div块的同时新建一个div块，并进入新div块中读取元素，直到读入\</div\>，退出当前div块并返回至父div块。
  ![](img/structure.svg)

#### 布局对齐：

- 每次退出当前div块时执行对齐操作(即修改pos)，若有子div块，则利用递归对齐子div块的元素。

#### 输出：

- 根据元素的pos输出至result中，如果元素为p或h，在StyleMem中记录样式，最后一并输出。
