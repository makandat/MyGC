/* test_gc.c */
#include "my_gc.h"

int main() {
  GcRoot* gc = gc_new();
  printf("   == test_gc ===\ncount = %d, first = %p, last = %p\n", gc->count, gc->first, gc->last);
  // ヒープ上の double 配列を追加
  size_t size = sizeof(double) * 100;
  void* data = malloc(size);
  memset(data, 0, size);
  gc_append(gc, data, size, -1);
  gc_dump(gc);
  // GC 内のデータを確認する。
  double *p = data;
  *p = 123.45;
  double* f = gc->first->data;
  printf("data = %f\n", *f);
  // スタック上の文字列を追加
  char *s1 = "0123456789";
  gc_append(gc, s1, strlen(s1), 0);
  printf("data = %s\n", (char*)gc->last->data);
  gc_dump(gc);
  // スタック上の整数を追加
  int n = 100;
  gc_append(gc, &n, 4, 1);
  int* pd = (int*)gc->last->data;
  printf("data = %d\n", *pd);
  gc_dump(gc);
  // セルを解放
  gc_free_cell(gc, &n);
  puts("gc_free_cell n");
  // スタック上の文字列を追加
  char *s2 = "!ABCDEF";
  gc_append(gc, s2, strlen(s2), 0);
  gc_dump(gc);
  // GC をすべて解放
  gc_free_all(gc, TRUE);
  gc_dump(gc);
  // ヒープ上の文字列を追加
  size_t len = 64;
  data = malloc(len);
  memset(data, 0x40, len);
  *((char*)data + len - 1) = '\0';
  gc_append(gc, data, 100, -1);
  printf("%s\n", (char*)gc->first->data);
  // GCリストにセルが１個しかないときに解放
  gc_free_cell(gc, data);
  // 4つのセルを追加
  puts("append 4 cells.");
  int* v1 = calloc(3, sizeof(int));
  int* v2 = calloc(3, sizeof(int));
  int* v3 = calloc(3, sizeof(int));
  int* v4 = calloc(3, sizeof(int));
  gc_append(gc, v1, 3, 0);
  gc_inc_ref(gc, v1); // v1 の参照カウントを増やす。
  gc_append(gc, v2, 3, 0);
  gc_append(gc, v3, 3, 0);
  puts("gc_inc_ref(gc, v3) 2回");
  gc_inc_ref(gc, v3); // v3 の参照カウントを増やす。
  gc_inc_ref(gc, v3); // v3 の参照カウントを増やす。
  gc_append(gc, v4, 3, 0);
  gc_free_cell(gc, v4);  // v4 を解放
  gc_free_cell(gc, v2);  // v2 を解放
  gc_dump(gc); // ダンプ
  puts("v1.count - 2");
  gc_dec_ref(gc, v1); // v1 の参照カウントを減らす。
  gc_dec_ref(gc, v1); // v1 の参照カウントを減らす。
  gc_dump(gc);
  puts("gc_free_all(gc, FALSE)");
  gc_free_all(gc, FALSE);
  gc_dump(gc);
  puts("gc_dec_ref(gc, v3)");
  gc_dec_ref(gc, v3);
  gc_dump(gc);
  puts("gc_free_all(gc, TRUE)");
  gc_free_all(gc, TRUE);
  gc_dump(gc);
  puts("Done.");
  return 0;
}
