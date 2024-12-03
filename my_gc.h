#ifndef MY_GC_H
#define MY_GC_H
/* my_gc.h */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef BOOL
#define BOOL  int
#define TRUE  1
#define FALSE 0
#endif

/* セルの定義 */
typedef struct tagGcCell {
  struct tagGcCell* next;  // 次のセル
  void* data;     // データ
  size_t size;    // データのバイト数
  int data_type;  // データ種別
  int count;      // 参照カウント
} GcCell;

/* ルートの定義 */
typedef struct tagGcRoot {
  struct tagGcCell* first;  // 先頭のセル
  struct tagGcCell* last;  // 最後のセル
  int count;  // セルの数
} GcRoot;

/* 関数のプロトタイプ */
extern GcRoot* gc_new();  // GcRoot を初期化する。
extern void* gc_append(GcRoot* root, void* data, size_t size, int data_type); // セルを追加する。
extern void gc_inc_ref(GcRoot* root, void* data);  // 参照カウントを増加させる。
extern void gc_dec_ref(GcRoot* root, void* data);  // 参照カウントを減少させる。カウントが0になったらそのセルは削除される。
extern void gc_free_cell(GcRoot* root, void* data);  // セルを解放する。(Cell.data_type < 0 のときはデータも解放)
extern void gc_free_all(GcRoot* root, BOOL force); // すべてのリソースを解放する。(Cell.data_type < 0 のときはデータも解放)
extern void gc_dump(GcRoot* root);  // GCリストをダンプ表示する。
extern GcCell* gc_find_cell(GcRoot* root, void* data);  // セルを検索する。


#endif
