# datAlloc

OSやCPUを問わず、あらゆる環境で動作する動的メモリ確保。

## I/F

詳細は [datAlloc.h](src/datAlloc/datAlloc.h) のコメントを参照。

`dataMalloc()`  
動的メモリ確保。使用方法はC標準ライブラリのmalloc()と同様。  

`dataFree()`  
dataMalloc()で確保したメモリの開放。使用方法はC標準ライブラリのfree()と同様。

`getHeapReport()`  
ヒープ領域の使用状況を取得する。  
コンパイルオプション「DA_DEBUG」が有効な場合のみ使用可。  

## コンパイルスイッチ

`DA_HEAP_SIZE`  
ヒープ代替領域のサイズ設定。デフォルトは2048(Byte)。  
※ datAllocは記憶領域の自動拡張機能を持たない。最大ヒープ使用量を調査して容量が枯渇しない値を設定すること。

`DA_ALIGNMENT`  
アライメントの設定。デフォルトは8。

`DA_DEBUG`  
datAllocのデバッグ機能を有効化する。