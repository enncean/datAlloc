#ifndef DATALLOC_DATALLOC_H
#define DATALLOC_DATALLOC_H

#include <stdlib.h>

#ifdef DA_DEBUG

typedef struct TagHeapDataReport
{
	// 確保されているアイテムの数
	unsigned int usedCount;
	// 使用中の領域(管理領域を含む)
	unsigned int usedSize;
	// ヒープ領域のサイズ
	unsigned int memorySize;
	// ヒープ領域
	unsigned const char* heapBuffer;
}HeapDataReport;

/**
 * @brief 動的メモリ用の領域の状態をレポートする。
 *
 * @param report レポート内容の格納先
 */
extern void getHeapReport (HeapDataReport* report);

#endif

/**
 * @brief 動的メモリを確保する。
 *
 * @param size 確保するメモリのサイズ
 * @return 確保されたメモリのアドレス
 */
extern void* dataMalloc (size_t size);

/**
 * @brief 確保した動的メモリを開放する。
 *
 * @param ptr 開放するメモリのアドレス
 */
extern void dataFree (void* ptr);

#endif //DATALLOC_DATALLOC_H
