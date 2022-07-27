﻿// © 2020-2022 Uniontech Software Technology Co.,Ltd.

#include "TermAccess.h" // for sfmt, ystdex::sfmt, TryAccessLeafAtom, IsTyped,
//	Unilang::Nonnull;
#include "Exception.h" // for ListTypeError, TypeError, ValueCategoryMismatch;
#include <ystdex/deref_op.hpp> // for ystdex::call_value_or;
#include <ystdex/functional.hpp> // for ystdex::compose, std::mem_fn,
//	ystdex::invoke_value_or;
#include "Context.h" // for complete Environment;

namespace Unilang
{

string
TermToString(const TermNode& term)
{
	if(const auto p = TermToNamePtr(term))
		return *p;
	return sfmt<string>("#<unknown{%zu}:%s>", term.size(),
		term.Value.type().name());
}

string
TermToStringWithReferenceMark(const TermNode& term, bool has_ref)
{
	auto term_str(TermToString(term));

	return has_ref ? "[*] " + std::move(term_str) : std::move(term_str);
}

TermTags
TermToTags(TermNode& term)
{
	AssertReferentTags(term);
	return ystdex::call_value_or(ystdex::compose(GetLValueTagsOf,
		std::mem_fn(&TermReference::GetTags)),
		TryAccessLeafAtom<const TermReference>(term), term.Tags);
}


#if Unilang_CheckTermReferenceIndirection
TermNode&
TermReference::get() const
{
	if(r_env.GetAnchorPtr() && r_env.GetPtr().lock())
		return term_ref.get();
	throw InvalidReference("Invalid reference found on indirection, probably"
		" due to invalid context access by a dangling reference.");
}
#endif


TermNode
PrepareCollapse(TermNode& term, const shared_ptr<Environment>& p_env)
{
	return IsTyped<TermReference>(term) ? term : Unilang::AsTermNode(
		term.get_allocator(), TermReference(p_env->MakeTermTags(term), term,
		Unilang::Nonnull(p_env)));
}


bool
IsReferenceTerm(const TermNode& term)
{
	return bool(TryAccessLeafAtom<const TermReference>(term));
}

bool
IsUniqueTerm(const TermNode& term)
{
	return ystdex::invoke_value_or(&TermReference::IsUnique,
		TryAccessLeafAtom<const TermReference>(term), true);
}

bool
IsModifiableTerm(const TermNode& term)
{
	return ystdex::invoke_value_or(&TermReference::IsModifiable,
		TryAccessLeafAtom<const TermReference>(term),
		!bool(term.Tags & TermTags::Nonmodifying));
}

bool
IsBoundLValueTerm(const TermNode& term)
{
	return ystdex::invoke_value_or(&TermReference::IsReferencedLValue,
		TryAccessLeafAtom<const TermReference>(term));
}

bool
IsUncollapsedTerm(const TermNode& term)
{
	return ystdex::call_value_or(ystdex::compose(IsReferenceTerm,
		std::mem_fn(&TermReference::get)),
		TryAccessLeafAtom<const TermReference>(term));
}

} // namespace Unilang;

