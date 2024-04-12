
#pragma once

#include "CoreMinimal.h"
#include "OctreeWay/OctreeSystem/OctreeObject.h"
#include "OctreeWay/Utils/Morton.h"


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
	bool Add(uint64 Key, TObjectPtr<UOctreeObject> Object, bool InputWithoutUnique = false)
	{
		if (!InputWithoutUnique)
		{
			for (int32 Index = 0; Index < Data.Num(); ++Index)
			{
				const FUint64AndObjectPtr& Elem = Data[Index];
				if (Elem.Key == Key)
				{
					return false; // Ключ уже существует.
				}
			}
		}

		Data.Add(FUint64AndObjectPtr(Key, Object)); // Добавляем новый элемент, если ключ уникальный.
		bIsSorted = false; // Пометить массив как неотсортированный.
		return true;
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

	// Переопределение метода FindClosest для использования улучшенного линейного поиска
	TObjectPtr<UOctreeObject> FindClosest(const uint64 Key)
	{
		if (Data.IsEmpty())
		{
			return nullptr; // Если массив пуст, возвращаем nullptr.
		}

		TObjectPtr<UOctreeObject> ClosestObject = nullptr;
		uint64 MinDifference = UINT64_MAX; // Инициализируем с максимально возможным различием.

		for (const auto& Elem : Data)
		{
			uint64 CurrentDifference = FMath::Abs(static_cast<int64>(Elem.Key) - static_cast<int64>(Key));

			// Обновляем ближайший объект и минимальную разницу, если находим меньшее значение.
			if (CurrentDifference < MinDifference)
			{
				MinDifference = CurrentDifference;
				ClosestObject = Elem.Value;
			}
		}

		return ClosestObject; // Возвращаем объект с минимальной разницей.
	}





private:
	TArray<FUint64AndObjectPtr> Data;
	bool bIsSorted = false; // Флаг, показывающий отсортирован ли массив.
};