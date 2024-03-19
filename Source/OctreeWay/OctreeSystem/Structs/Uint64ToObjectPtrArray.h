
#pragma once

#include "CoreMinimal.h"
#include "OctreeWay/OctreeSystem/OctreeObject.h"


class UOctreeObject;

// Структура для хранения пары значений.
struct FUint64AndObjectPtr
{
	uint64 Key;
	TObjectPtr<UOctreeObject> Value;

	// Конструктор для упрощения создания экземпляров.
	FUint64AndObjectPtr(uint64 InKey, TObjectPtr<UOctreeObject> InValue)
		: Key(InKey), Value(InValue) {}

	// Оператор для сравнения, необходимый для сортировки и бинарного поиска.
	bool operator<(const FUint64AndObjectPtr& Other) const
	{
		return Key < Other.Key;
	}
};

// Класс для управления массивом пар значений.
class FUint64ToObjectPtrArray
{
public:
	// Добавление новой пары значений с проверкой уникальности ключа.
	bool Add(uint64 Key, TObjectPtr<UOctreeObject> Object)
	{
		if (!Find(Key))
		{
			Data.Emplace(Key, Object);
			bIsSorted = false; // Пометить массив как неотсортированный.
			return true;
		}
		return false;
	}

	// Сортировка массива по ключам.
	void Sort()
	{
		if (!bIsSorted)
		{
			Data.Sort();
			bIsSorted = true;
		}
	}

	// Бинарный поиск по ключу.
	TObjectPtr<UOctreeObject> Find(const uint64 Key)
	{
		if (!bIsSorted)
		{
			Sort(); // Убедиться, что данные отсортированы перед поиском.
		}

		int32 Low = 0;
		int32 High = Data.Num() - 1;

		while (Low <= High)
		{
			const int32 Mid = (Low + High) / 2;
			if (Data[Mid].Key == Key)
			{
				return Data[Mid].Value; // Возвращаем указатель на найденный объект.
			}
			else if (Data[Mid].Key < Key)
			{
				Low = Mid + 1;
			}
			else
			{
				High = Mid - 1;
			}
		}

		return nullptr; // Если ключ не найден, возвращаем nullptr.
	}

	// Поиск TObjectPtr<UOctreeObject> с ближайшим uint64 Key к заданному параметру.
	TObjectPtr<UOctreeObject> FindClosest(const uint64 Key)
	{
		if (Data.Num() == 0)
		{
			return nullptr; // Массив пуст, возвращаем nullptr.
		}

		if (!bIsSorted)
		{
			Sort(); // Убедиться, что данные отсортированы перед поиском.
		}

		int32 Low = 0;
		int32 High = Data.Num() - 1;
		int32 ClosestIndex = -1;

		while (Low <= High)
		{
			const int32 Mid = (Low + High) / 2;
			if (Data[Mid].Key == Key)
			{
				return Data[Mid].Value; // Точное совпадение найдено.
			}

			// Обновляем ближайший индекс.
			const uint64 Difference = (Data[Mid].Key > Key) ? Data[Mid].Key - Key : Key - Data[Mid].Key;
			const uint64 ClosestDifference = (ClosestIndex == -1) ? UINT64_MAX : (Data[ClosestIndex].Key > Key) ? Data[ClosestIndex].Key - Key : Key - Data[ClosestIndex].Key;

			if (ClosestIndex == -1 || Difference < ClosestDifference) {
				ClosestIndex = Mid;
			}

			if (Data[Mid].Key < Key)
			{
				Low = Mid + 1;
			}
			else
			{
				High = Mid - 1;
			}
		}

		// После выхода из цикла, ClosestIndex должен указывать на элемент с ближайшим ключом.
		if (ClosestIndex != -1)
		{
			return Data[ClosestIndex].Value;
		}

		return nullptr; // Не должны сюда попасть, но на всякий случай.
	}


private:
	TArray<FUint64AndObjectPtr> Data;
	bool bIsSorted = false; // Флаг, показывающий отсортирован ли массив.
};