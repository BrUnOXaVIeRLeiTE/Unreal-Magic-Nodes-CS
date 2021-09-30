//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MagicNodeSharp.h"
#include "MagicNodeSharpKismet_Shared.h"

#include "KCS_NodeStyle.h"
#include "KCS_WhiteSpaceTextRun.h"

#include "Styling/SlateTypes.h"
#include "Framework/Text/SyntaxTokenizer.h"
#include "Framework/Text/SyntaxHighlighterTextLayoutMarshaller.h"

#include "AssetRegistryModule.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class FTextLayout;
class UMagicNodeSharpSource;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MAGICNODESHARPKISMET_API FTextSyntaxHighlighter : public FSyntaxHighlighterTextLayoutMarshaller {
public:
	void SetScriptSource(UMagicNodeSharpSource* InScriptSource) {ScriptSource=InScriptSource;}
public:
	virtual ~FTextSyntaxHighlighter();
public:
	struct FSyntaxTextStyle {
		FTextBlockStyle TypeTextStyle;
		FTextBlockStyle EventTextStyle;
		FTextBlockStyle MacroTextStyle;
		FTextBlockStyle ClassTextStyle;
		FTextBlockStyle NormalTextStyle;
		FTextBlockStyle StringTextStyle;
		FTextBlockStyle NumberTextStyle;
		FTextBlockStyle IllegalTextStyle;
		FTextBlockStyle KeywordTextStyle;
		FTextBlockStyle CommentTextStyle;
		FTextBlockStyle OperatorTextStyle;
		FTextBlockStyle PropertyTextStyle;
		FTextBlockStyle FunctionTextStyle;
		FTextBlockStyle ContainerTextStyle;
		FTextBlockStyle ProcessorTextStyle;
		//
		FSyntaxTextStyle()
			: TypeTextStyle(FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.SyntaxHighlight.Type"))
			, EventTextStyle(FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.SyntaxHighlight.Event"))
			, MacroTextStyle(FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.SyntaxHighlight.Macro"))
			, ClassTextStyle(FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.SyntaxHighlight.Class"))
			, NormalTextStyle(FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.SyntaxHighlight.Normal"))
			, StringTextStyle(FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.SyntaxHighlight.String"))
			, NumberTextStyle(FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.SyntaxHighlight.Number"))
			, IllegalTextStyle(FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.SyntaxHighlight.Illegal"))
			, KeywordTextStyle(FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.SyntaxHighlight.Keyword"))
			, CommentTextStyle(FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.SyntaxHighlight.Comment"))
			, OperatorTextStyle(FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.SyntaxHighlight.Operator"))
			, PropertyTextStyle(FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.SyntaxHighlight.Property"))
			, FunctionTextStyle(FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.SyntaxHighlight.Function"))
			, ContainerTextStyle(FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.SyntaxHighlight.Container"))
			, ProcessorTextStyle(FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.SyntaxHighlight.Processor"))
		{}//
	};//
public:
	static TSharedRef<FTextSyntaxHighlighter>Create(const FSyntaxTextStyle &InSyntaxStyle);
protected:
	enum class EParserState : uint8 {
		None,
		LookingForChar,
		LookingForString,
		LookingForMultiLineComment,
		LookingForSingleLineComment
	};//
	//
	FSyntaxTextStyle SyntaxTextStyle;
protected:
	bool IsNamespace(const FString &Word) const;
	bool IsClassType(const FString &Word) const;
	bool IsFieldType(const FString &Word) const;
	bool IsPropertyType(const FString &Word) const;
	bool IsFunctionType(const FString &Word) const;
protected:
	bool IsAutoComplete(const FString &Word) const;
	bool IsAutoCompleteTree(const FString &Word) const;
protected:
	const FString FilterKeyword(const FString &Keyword) const;
	uint32 CountChars(const TCHAR &CH, const FString &Word) const;
	const TArray<FString>ParseAutoCompleteTree(const FString &Word) const;
protected:
	virtual void ParseTokens(const FString &Source, FTextLayout &Layout, TArray<FSyntaxTokenizer::FTokenizedLine>TokenizedLines) override;
	void ParseToken(const FString &Source, const FString &Context, const FString &Keyword, const FSyntaxTokenizer::FToken &Token, FRunInfo &Info, FTextBlockStyle &TextStyle, EParserState &ParserState);
protected:
	FTextSyntaxHighlighter(TSharedPtr<FSyntaxTokenizer>InTokenizer, const FSyntaxTextStyle &InSyntaxTextStyle)
		: FSyntaxHighlighterTextLayoutMarshaller(MoveTemp(InTokenizer))
		, SyntaxTextStyle(InSyntaxTextStyle)
	{}///
private:
	TWeakObjectPtr<UMagicNodeSharpSource>ScriptSource;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////