#pragma once

namespace Faerie
{
	class BitMatrix
	{
	public:
		int32 Width;
		int32 Height;

		void Init(int32 InWidth, int32 InHeight)
		{
			Width = InWidth;
			Height = InHeight;
			// Calculate how many uint32s we need to store all bits
			int32 NumInts = (Width * Height + 31) / 32;
			Data.SetNum(NumInts, false);
		}

		void Set(int32 X, int32 Y, bool Value)
		{
			int32 Index = Y * Width + X;
			int32 IntIndex = Index / 32;
			int32 BitIndex = Index % 32;
        
			if (Value)
			{
				Data[IntIndex] |= (1u << BitIndex);
			}
			else
			{
				Data[IntIndex] &= ~(1u << BitIndex);
			}
		}

		bool Get(int32 X, int32 Y) const
		{
			int32 Index = Y * Width + X;
			int32 IntIndex = Index / 32;
			int32 BitIndex = Index % 32;
			return (Data[IntIndex] & (1u << BitIndex)) != 0;
		}

		void Transpose();
		void Reverse();
		
		private:
			TArray<uint32> Data;
	};
}
