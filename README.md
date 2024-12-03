<!DOCTYPE html>
<html lang="ja">
<head>
 <meta charset="utf-8" />
 <meta name="viewport" content="width=device-width,initial-scale=1" />
 <title>C GC Library</title>
 <!-- BootstrapのCSS読み込み -->
 <link href="https://cdn.jsdelivr.net/npm/bootstrap/dist/css/bootstrap.min.css" rel="stylesheet">
 <style>
  h2 {
    color:crimson;
  }
  pre {
    border: solid thin gray;
    border-radius: 6px;
    padding: 4px;
  }
  .sample {
     color: darkred;
     margin-left:3%;
   }
 </style>
</head>

<body>
 <!-- ヘッダー -->
 <header class="container">
  <h1 class="text-center p-5 border border-warning rounded">C GC Library</h1>
  <p class="text-center"><a href="/">HOME</a></p>
 </header>

 <!-- 本文 -->
 <article class="container">
  <section class="row">
   <h2>概要</h2>
   <p>C 言語では malloc や calloc関数などで取得したメモリブロックはヒープ上にあって、free 関数で解放しないと自動的に解放されない。そのため、
ループ内などで繰り返し malloc 関数などを繰り返し使うとヒープメモリが圧迫され、最終的に例外が発生する。</p>
   <p>手動でこれらのメモリブロックを解放するのは単純ではない。つまり、このメモリブロックを使っている処理が１つだけとは限らないためである。
Python や C# などでは Garbage Collection (GC) という方式で、これらのメモリブロックを解放している。これは、そのブロックの参照カウントが、
0 になったら自動的に解放するというものである。</p>
   <p>C 言語ではそのような仕組みを備えていないため、手動によるメモリブロックの解放が必要になるが、この Garbage Collection 
のようなメモリブロックのリストを用意すれば、完全な手動処理よりメモリブロックの管理が簡単になる。<p>
   <p>このライブラリはそのようなメモリブロックの管理リストを提供する。</p>
  </section>
  <br />
  <section class="row">
   <h2>使用方法</h2>
   <h3>ビルド方法</h3>
   <p>必要なライブラリは glibc (標準Cライブラリ) のみである。ソースは、my_gc.h, my_gc.c のみである。以下に makefile の例を示す。</p>
   <pre>SRC = my_gc.c my_gc.h
TARGET = bin/my_gc.so
OPT = -g -shared -std=c11
$(TARGET) : $(SRC)
	gcc -o $@ $(OPT) $(SRC)</pre>
   <p>この makefile によりビルドを行うと ./bin/my_gc.so というファイルができるので、これをリンクしてプログラムをビルドする。下にその例を示す。
    <pre>make -s
gcc -o bin/test_gc -std=c11 -g test_gc.c bin/my_gc.so
echo "更新されました。 bin/test_gc."</pre>
   <br />
   <h3>my_gc.so 関数の使用</h3>
   <p>次の手順でmy_gc 関数を使用する。</p>
   <ul>
    <li>gc_new 関数で GC リストを作成する。</li>
    <li>gc_append 関数で GC リストに GC セルを追加する。gc_append 関数は作成した GC セルの data (void*ポインタ) を返す。</li>
    <li>セルを追加したとき、データを初期化しなかったときは、データを初期化する。これは、gc_append 関数が返したポインタは GcCell の
data (void* 型) である。このデータの長さは gc_append 関数で指定したバイト数である。</li>
    <li>他の関数に GC セルまたは GC セルデータを渡す時は、gc_inc_ref 関数で参照カウントを増やす。</li>
    <li>他の関数から戻った時は、gc_dec_ref 関数で参照カウントを減らす。</li>
    <li>不要になった GC セルは gc_dec_ref 関数で参照カウントを 0 にする。(参照カウントが 0 の GC セルは自動で削除される。)</li>
    <li>処理終了前に gc_free_all 関数を実行してすべての GC セルを解放する。</p>
   </ul>
   <br />
   <h3>gc_append 関数について</h3>
   <p>gc_append 関数は次のような形式になっている。data は GC セルの data として設定される。ただし、 data が NULL のときは
関数内で size に基づいてメモリブロックを確保する。size はメモリブロックのバイト数で data が NULL でないときも関数に渡す必要がある。</p>
   <p>data_type はユーザが定義するコードで data の型を示すのに使用できる。ただし、負数の場合はセルが解放されるとき、data が指す
メモリブロックも同時に解放される。</p>
   <p class="sample">void* gc_append(GcRoot* root, void* data, size_t size, int data_type)</p>
   <br />
   <h3>gc_free_all 関数について</h3>
   <p>gc_free_all 関数は以下のような形式をしている。force が TRUE のときは参照カウントに関わらずすべての CG セルを解放する。
forcr=FALSE のときは、参照カウントが 1 以下のすべての GC セルを解放する。</p>
   <p>force=FALSE のとき、参照カウントが 2 以上のセルに対しては何もしない。よって、必要に応じて gc_dec_ref 関数も併用する。</p>
   <p class="sample">void gc_free_all(GcRoot* root, BOOL force)</p>
   <br />
   <h3>サンプル</h3>
   <pre>/* test_gc.c */
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
</pre>
  </section>
  <br />
  <section class="row">
   <h2>関数の詳細</h2>
   <dl>
    <dt>GcRoot* gc_new()</dt>
    <dd>GC リストを初期化する。<br />(例) GcRoot* gc = gc_new();</dd>
   </dl>

   <dl>
    <dt>void* gc_append(GcRoot* root, void* data, size_t size, int data_type)</dt>
    <dd>GC リストにセルを追加する。
      <ul>
        <li>root: gc_new() の戻り値</li>
        <li>data: データへのポインタ。NULL の場合は、関数内で領域を割り当てる。</li>
        <li>size: data のバイト数</li>
        <li>data_type: データの型を識別するためのユーザが定義するコード。ただし、負の場合はセルを削除するときに data も削除される。</li>
        <li>戻り値: data (データへのポインタ)</li>
      <ul>
    </dd>
   </dl>

   <dl>
    <dt>void gc_inc_ref(GcRoot* root, void* data)</dt>
    <dd>data で指定したセルの参照カウントを増やす。</dd>
   </dl>

   <dl>
    <dt>void gc_dec_ref(GcRoot* root, void* data)</dt>
    <dd>data で指定したセルの参照カウントを減らす。その結果、参照カウントが 0 になったときは、そのセルも解放される。</dd>
   </dl>

   <dl>
    <dt>void gc_free_cell(GcRoot* root, void* data)</dt>
    <dd>data で指定したセルを解放する。そのセルの data_type が負の場合は、セルだけでなく data 自身も解放される。</dd>
   </dl>

   <dl>
    <dt>void gc_free_all(GcRoot* root, BOOL force)</dt>
    <dd>参照カウントが 1 以下の場合、それらのセルを解放する。ただし、force=TRUE のときはすべてのセルを解放する。</dd>
   </dl>


   <dl>
    <dt>void gc_dump(GcRoot* root)</dt>
    <dd>GC リストの内容をダンプ表示する。デバッグ用。</dd>
   </dl>


   <dl>
    <dt>GcCell* gc_find_cell(GcRoot* root, void* data)</dt>
    <dd>GC リスト内で data で指定したセルへのポインタを得る。主に内部用。</dd>
   </dl>

  </section>
 </article>
 
 <!-- フッター -->
 <footer class="container">
  <p class="text-center mt-4"><a href="#top">TOP</a></p>
  <p>&nbsp;</p>
 </footer>
 <!-- BootstrapのJS読み込み -->
 <script src="https://cdn.jsdelivr.net/npm/bootstrap/dist/js/bootstrap.bundle.min.js"></script>
</body>
</html>
