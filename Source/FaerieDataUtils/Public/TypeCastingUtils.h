// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

namespace Type
{
	template <typename TOut, typename TIn>
	static [[nodiscard]] TOut Cast(TIn&& In)
	{
		return *reinterpret_cast<TOut*>(&In);
	}
}