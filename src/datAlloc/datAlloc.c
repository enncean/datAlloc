#include "datAlloc.h"

#ifndef DA_HEAP_SIZE
#define DA_HEAP_SIZE	2048
#endif

#ifndef DA_ALIGNMENT
#define DA_ALIGNMENT	8
#endif

// 符号無し32bit(管理領域のパラメータ用)
typedef unsigned int U32;

static U32 readFromHeapU32 (unsigned int offset);
static void writeToHeapU32 (unsigned int offset, U32 data);

#define DA_HEADER_SIZE			8	// 管理領域のサイズ ( NxtHeader(4) + DataSize(4) = 8Byte )
#define DA_HEADER_OFFSET_SIZE	4	// 管理領域の先頭からサイズ情報までのオフセット

/**
 * @brief ヒープの代替領域
 *
 * <br>
 * <b>ヒープの使い方について</b><br>
 * <br>
 * メモリの動的確保時に格納される情報は以下の通り:<br>
 * <pre>
 * |+0|+1|+2|+3|+4|+5|+6|+7|+8|+9|+A|..|
 * | Header                | Data ...  |
 * | NxtHeader | DataSize  |           |
 * </pre>
 * <br>
 * Header : 確保した領域の管理領域で、領域1つ毎に付与される。<br>
 * <ul>
 * <li>NxtHeader : 次のHeaderの先頭Index。次が存在しない場合、バッファの先頭(0)を指す。</li>
 * <li>DataSize : 確保した領域のサイズ。</li>
 * </ul>
 * <br>
 * 補足 :<br>
 * <ul>
 * <li>daHeapの先頭には最初のHeaderを指すために、NxtHeaderのみ(4Byte)の管理領域が配置される。<br>
 * これが先頭要素を返す場合、動的に確保された領域は存在しないことを示す。</li>
 * <li>領域1つあたりの消費メモリは要求されたサイズ+ヘッダ部分(8Byte)となるが、<br>
 * アライメントや断片化の関係でこれより増加する可能性もある。</li>
 * </ul>
 */
static unsigned char daHeap[DA_HEAP_SIZE] = { 0x00U };

/* ==== 外部公開関数 ==== */

#ifdef DA_DEBUG
extern void getHeapReport (HeapDataReport* report)
{
	if (report != NULL)
	{
		// 次に読み込むヘッダ位置。初期値(0x00000000U)は先頭要素のヘッダ位置が格納されている場所。
		U32 readingOffset = 0x00000000U;
		
		report->usedCount = 0;
		report->usedSize = 0;
		report->memorySize = sizeof(daHeap);
		report->heapBuffer = daHeap;
		
		// 最初の要素の場所を取得
		readingOffset = readFromHeapU32(readingOffset);
		
		while (readingOffset != 0x00000000U)
		{
			// 現在指しているヘッダが管理している情報の取得
			report->usedCount++;
			report->usedSize += DA_HEADER_SIZE + readFromHeapU32(readingOffset + DA_HEADER_OFFSET_SIZE);
			// 次のヘッダ位置取得
			readingOffset = readFromHeapU32(readingOffset);
		}
	}
}
#endif // DA_DEBUG

extern void* dataMalloc (size_t size)
{
	// 次に読み込むヘッダ位置。初期値(0x00000000U)は先頭要素のヘッダ位置が格納されている場所。
	U32 readingOffset = 0x00000000U;
	
	unsigned char hasData;					// 1つ以上のデータが存在するか否か
	U32 candidateIndex = 0x00000000U;		// 確保する場所候補のIndex
	U32 candidatePrevIndex = 0x00000000U;	// 確保する場所候補の1つ前のIndex
	U32 candidateSize = 0xFFFFFFFFU;		// 確保する場所候補の、次の領域までのサイズ
	
	// アライメントへの対応
	// 要求されたサイズが DA_ALIGNMENT の倍数ではない場合、倍数になるようにサイズを増やす。
	if (size % DA_ALIGNMENT != 0)
	{
		size += DA_ALIGNMENT - (size % DA_ALIGNMENT);
	}
	
	// ==== 次に使用する領域を探す ====
	readingOffset = readFromHeapU32(readingOffset);
	hasData = (readingOffset != 0x00000000U);
	while (readingOffset != 0x00000000U)
	{
		// 次の領域のヘッダ位置取得
		U32 nextHeaderIndex = readFromHeapU32(readingOffset);
		// この領域で確保しているサイズを取得
		U32 allocatedSize = readFromHeapU32(readingOffset + DA_HEADER_OFFSET_SIZE);
		// この領域と次の領域(無いならdaHeap末尾)までの間にある空き領域の算出
		U32 emptySizeForNext =
				((nextHeaderIndex == 0) ? DA_HEAP_SIZE : nextHeaderIndex) -
				(readingOffset + DA_HEADER_SIZE + allocatedSize);
		
		if (((size + DA_HEADER_SIZE) <= emptySizeForNext) && (emptySizeForNext < candidateSize))
		{
			candidateIndex = readingOffset + DA_HEADER_SIZE + allocatedSize;
			candidatePrevIndex = readingOffset;
			candidateSize = emptySizeForNext;
		}
		
		readingOffset = nextHeaderIndex;
	}
	
	// ==== 領域の割り当て ====
	if (!hasData)
	{
		// 使用中の領域が1つも無かった場合、先頭ヘッダの次から割り当てる
		
		// 使用中の領域が無いパターンではサイズチェックが走らないため、ここでヒープ代替領域に収まるかチェックする。
		if (size <= (DA_HEAP_SIZE - DA_HEADER_OFFSET_SIZE - DA_HEADER_SIZE))
		{
			candidateIndex = DA_HEADER_OFFSET_SIZE;
		}
	}
	if (candidateIndex != 0x00000000U)
	{
		// 領域確保/次の領域のIndex
		writeToHeapU32(candidateIndex, readFromHeapU32(candidatePrevIndex));
		// 領域確保/この領域で確保しているサイズ
		writeToHeapU32(candidateIndex + DA_HEADER_OFFSET_SIZE, size);
		
		// 1つ前の領域が指す、次の領域のIndexを更新
		writeToHeapU32(candidatePrevIndex, candidateIndex);
		
		return &daHeap[candidateIndex + DA_HEADER_SIZE];
	}
	
	return NULL;
}

extern void dataFree (void* ptr)
{
	if (ptr != NULL)
	{
		// 次に読み込むヘッダ位置
		U32 readingOffset = readFromHeapU32(0x00000000U);
		// readingOffsetの1つ前の領域
		U32 prevReadOffset = 0x00000000U;
		
		while (readingOffset != 0x00000000U)
		{
			if (ptr == &daHeap[readingOffset + DA_HEADER_SIZE])
			{
				// 1つ前の領域と次の領域を紐付けることで、指定された領域を管理対象から外す
				writeToHeapU32(prevReadOffset, readFromHeapU32(readingOffset));
				break;
			}
			
			prevReadOffset = readingOffset;
			readingOffset = readFromHeapU32(readingOffset);
		}
	}
}

/* ==== 内部関数 ==== */

#define BITOFFSET_1BYTE	8
#define BITOFFSET_2BYTE	16
#define BITOFFSET_3BYTE	24

static U32 readFromHeapU32 (unsigned int offset)
{
	return ((U32)daHeap[offset]) |
		   ((U32)daHeap[offset + 1] << BITOFFSET_1BYTE) |
		   ((U32)daHeap[offset + 2] << BITOFFSET_2BYTE) |
		   ((U32)daHeap[offset + 3] << BITOFFSET_3BYTE);
}

static void writeToHeapU32 (unsigned int offset, U32 data)
{
	daHeap[offset] = (unsigned char)(data & 0xFFU);
	daHeap[offset + 1] = (unsigned char)((data >> BITOFFSET_1BYTE) & 0xFFU);
	daHeap[offset + 2] = (unsigned char)((data >> BITOFFSET_2BYTE) & 0xFFU);
	daHeap[offset + 3] = (unsigned char)((data >> BITOFFSET_3BYTE) & 0xFFU);
}