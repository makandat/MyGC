/* my_gc.c v1.0 */
#include "my_gc.h"

/* Root を初期化する。 */
GcRoot* gc_new() {
  GcRoot* root = malloc(sizeof(GcRoot));
  root->first = NULL; // 最初のセル
  root->last = NULL;  // 最後のセル
  root->count = 0;    // セルの数
  return root;
}

/* セルを追加する。
     root: gc_new() の戻り値
     data: データへのポインタ。NULL の場合は、関数内で領域を割り当てる。
     size: data のバイト数
     data_type: データの型を識別するためのユーザが定義するコード。ただし、負の場合はセルを削除するときに data も削除される。
     戻り値: data (データへのポインタ)
 */
void* gc_append(GcRoot* root, void* data, size_t size, int data_type) {
  GcCell* cell = malloc(sizeof(GcCell));
  cell->next = NULL;  // 次のセル
  cell->data = data;  // データへのポインタ
  if (cell->data == NULL) {
    // data が NULL のときは 0 クリアされたメモリを割り当てる。
    cell->data = calloc(size, 1);
  }
  cell->size = size;  // データのバイト数
  cell->data_type = data_type;  // データ種別
  cell->count = 1;  // 参照カウント
  // このセルをリストの最後に追加する。
  if (root->first == NULL) {
    // リストが空のとき
    root->first = cell;
    root->last = cell;
    root->count = 1;
  }
  else {
    // リストにセルがあるとき
    GcCell* last_cell = root->last;
    last_cell->next = cell;
    root->last = cell;
    root->count++;
  }
  return cell->data;
}

/* 参照カウントを増加させる。 */
void gc_inc_ref(GcRoot* root, void* data) {
  GcCell* cell = gc_find_cell(root, data);
  cell->count++;
}

/* 参照カウントを減少させる。 */
void gc_dec_ref(GcRoot* root, void* data) {
  GcCell* cell = gc_find_cell(root, data);
  if (cell == NULL)
    return;  // 存在しないセルのときはそのままもどる。
  cell->count--;
  if (cell->count <= 0) {
    // そのセルをリストから取り除く。
    gc_free_cell(root, data);
  }
}

/* セルを解放する。 
    root: gc_new の戻り値
    data: 解放するセルの data (ポインタ)
    戻り値: なし
*/
void gc_free_cell(GcRoot* root, void* data) {
  // リストが空なら何もしない。
  if (root->count == 0)
    return;
  // リストを走査する準備
  GcCell* current = root->first;
  GcCell* prev = root->first;
  if (current->data == data) {
    // 最初のでセルが解放する対象のとき
    if (current->next == NULL) {
      // 先頭のセルしかないとき、root を初期化する。
      root->count = 0;
      root->first = NULL;
      root->last = NULL;
    }
    else {
      // 続きのセルがあるとき、root->first を次のセルに付け替える。
      root->first = current->next;
      root->count--;  // 参照カウントを減らす。
    }
    // 現在のセルを解放
    if (current->data_type < 0)
      free(current->data);
    free(current);
  }
  else {
    // リストを最後まで走査する。
    int n = root->count;
    for (int i = 0; i < n; i++) {
      if (current->next == NULL) {
        // 最後のセルのとき
        if (current->data == data) {
          // data が一致したとき、前のセルの next を NULL にする。
          prev->next = NULL;
          // 参照カウントを減らす。
          root->count--;
          // セルを解放する。
          if (current->data_type < 0)
            free(current->data);
          free(current);
          break;          
        }
        else {
          // 最後のセルでも data が一致しないときは何もせずに戻る。
          return;
        }
      }
      else {
        // 最後のセルでないとき、次のセルへ移動する。
        prev = current;
        current = current->next;
        if (current->data == data) {
          // 解放するセルのとき、前のセルの next を次のセルへ付け替える。
          GcCell* next_cell = current->next;
          prev->next = next_cell;
          // 最後のセルなら root->last を前のセルにする。
          if (current == root->last) {
            root->last = prev;
          }
          root->count--;
          // セルを解放する。
          if (current->data_type < 0)
            free(current->data);
          free(current);
          break;
        }
      }
    }
  }
  // リストにセルがない場合は root を初期化する。
  if (root->count == 0) {
    root->first = NULL;
    root->last = NULL;
  }
}

/* 参照カウントが 1 以下のすべてのリソースを解放する。 
   root: gc_new() の戻り値
   force: 参照カウントに関わらず解放する。
*/
void gc_free_all(GcRoot* root, BOOL force) {
  // リストが空なら何もしない。
  if (root->count == 0)
    return;
  GcCell* current = root->first;
  // すべてのセルに対して行う。
  int n = root->count;
  for (int i = 0; i < n; i++) {
    if (current->data_type < 0)
      free(current->data);
    GcCell* next_cell = current->next;
    if (force != 0) { // 参照カウントに関わらず解放
      free(current);
      root->count--;
    }
    else if (current->count <= 1) { // 参照カウントが１または0なら解放
      free(current);
      root->count--;
    }
    current = next_cell;
  }
  // root を初期化する。
  if (root->count == 0) {
    root->first = NULL;
    root->last = NULL;
  }
}

/* リストをダンプ表示する。 */
void gc_dump(GcRoot* root) {
  puts("GC Dump");
  printf("cell count=%d, first=%p, last=%p\n", root->count, root->first, root->last);
  GcCell* p = root->first;
  if (p == NULL) {
    puts("GC Empty.");
    return;
  }
  for (int i = 0; i < root->count; i++) {
    printf(" index=%u, data=%p, size=%ld, data_type=%d, count=%d\n", i, p->data, p->size, p->data_type, p->count);
    p = p->next;
  }
}

/* セルを検索する。 
    root: gc_new() の戻り値
    data: 検索するセルのデータポインタ
    戻り値: 見つかった時はセルへのポインタ、見つからなかったときは NULL
*/
GcCell* gc_find_cell(GcRoot* root, void* data) {
  GcCell* p = root->first;
  for (int i = 0; i < root->count; i++) {
    if (p->data == data) {
      return p; // 見つかった時はセルへのポインタを返す。
    }
    p = p->next;
  }
  return NULL;  // セルが見つからなかったとき
}
