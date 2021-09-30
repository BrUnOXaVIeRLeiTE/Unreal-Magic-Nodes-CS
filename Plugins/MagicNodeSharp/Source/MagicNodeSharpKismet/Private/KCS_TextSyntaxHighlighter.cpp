//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
///		Copyright 2021 (C) Bruno Xavier B. Leite
///		Based on code sample by Epic Games (C) 1998-2020 Epic Games, Inc.
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "KCS_TextSyntaxHighlighter.h"
#include "KCS_MonoAnalyzer.h"

#include "AssetRegistryModule.h"

#include "Framework/Text/IRun.h"
#include "Framework/Text/ISlateRun.h"
#include "Framework/Text/TextLayout.h"
#include "Framework/Text/SlateTextRun.h"

#include "Misc/ExpressionParserTypes.h"
#include "Runtime/Core/Public/Async/TaskGraphInterfaces.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FTextSyntaxHighlighter::~FTextSyntaxHighlighter(){}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TSharedRef<FTextSyntaxHighlighter>FTextSyntaxHighlighter::Create(const FSyntaxTextStyle &InSyntaxStyle) {
	TArray<FSyntaxTokenizer::FRule>TokenizerRules;
	//
	for (const auto &Opetr : CS::Operators) {
		TokenizerRules.Emplace(FSyntaxTokenizer::FRule(Opetr));
	}///
	//
	return MakeShareable(new FTextSyntaxHighlighter(FSyntaxTokenizer::Create(TokenizerRules),InSyntaxStyle));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FTextSyntaxHighlighter::ParseTokens(const FString &Source, FTextLayout &Layout, TArray<FSyntaxTokenizer::FTokenizedLine>TokenizedLines) {
	TArray<FTextLayout::FNewLineData>ParsedLines;
	ParsedLines.Reserve(TokenizedLines.Num());
	//
	EParserState ParserState = EParserState::None;
	//
	for (const FSyntaxTokenizer::FTokenizedLine &TokenizedLine : TokenizedLines) {
		TSharedRef<FString>Runner = MakeShareable(new FString());
		TArray<TSharedRef<IRun>>Runs;
		FString LText{};
		//
		if (ParserState==EParserState::LookingForSingleLineComment) {
			ParserState=EParserState::None;
		}///
		//
		for (const FSyntaxTokenizer::FToken &Token : TokenizedLine.Tokens) {
			const FString TText = Source.Mid(Token.Range.BeginIndex,Token.Range.Len());
			//
			const FTextRange RunRange(Runner->Len(),Runner->Len()+TText.Len());
			const bool Whitespace = FString(TText).TrimEnd().IsEmpty();
			//
			FTextBlockStyle TextStyle = SyntaxTextStyle.NormalTextStyle;
			FRunInfo RunInfo(TEXT("KCS.SyntaxHighlight.Normal"));
			Runner->Append(TText);
			//
			if (Whitespace) {
				RunInfo.Name = TEXT("KCS.SyntaxHighlight.WhiteSpace");
				Runs.Add(FWhiteSpaceTextRun::Create(RunInfo,Runner,TextStyle,RunRange,4));
			} else {
				ParseToken(Source,LText,TText,Token,RunInfo,TextStyle,ParserState);
				Runs.Add(FSlateTextRun::Create(RunInfo,Runner,TextStyle,RunRange));
			}///
			//
			LText.Append(TText);
			LText.TrimStartInline();
		}///
		//
		ParsedLines.Emplace(MoveTemp(Runner),MoveTemp(Runs));
	}///
	//
	Layout.AddLines(ParsedLines);
}

void FTextSyntaxHighlighter::ParseToken(const FString &Source, const FString &Context, const FString &Keyword, const FSyntaxTokenizer::FToken &Token, FRunInfo &Info, FTextBlockStyle &TextStyle, EParserState &ParserState) {
	if (Token.Type==FSyntaxTokenizer::ETokenType::Syntax) {
		if (ParserState==EParserState::None && Keyword==TEXT("//")) {
			Info.Name = TEXT("KCS.SyntaxHighlight.Comment");
			TextStyle = SyntaxTextStyle.CommentTextStyle;
			ParserState = EParserState::LookingForSingleLineComment;
		} else if (ParserState==EParserState::None && Keyword==TEXT("/*")) {
			Info.Name = TEXT("KCS.SyntaxHighlight.Comment");
			TextStyle = SyntaxTextStyle.CommentTextStyle;
			ParserState = EParserState::LookingForMultiLineComment;
		} else if (ParserState==EParserState::LookingForMultiLineComment && Keyword==TEXT("*/")) {
			Info.Name = TEXT("KCS.SyntaxHighlight.Comment");
			TextStyle = SyntaxTextStyle.CommentTextStyle;
			ParserState = EParserState::None;
		} else if (ParserState==EParserState::None && Keyword==TEXT("\"")) {
			Info.Name = TEXT("KCS.SyntaxHighlight.String");
			TextStyle = SyntaxTextStyle.StringTextStyle;
			ParserState = EParserState::LookingForString;
		} else if (ParserState==EParserState::LookingForString && Keyword==TEXT("\"")) {
			Info.Name = TEXT("KCS.SyntaxHighlight.Normal");
			TextStyle = SyntaxTextStyle.StringTextStyle;
			ParserState = EParserState::None;
		} else if (ParserState==EParserState::None && Keyword==TEXT("\'")) {
			Info.Name = TEXT("KCS.SyntaxHighlight.String");
			TextStyle = SyntaxTextStyle.StringTextStyle;
			ParserState = EParserState::LookingForChar;
		} else if (ParserState==EParserState::LookingForChar && Keyword==TEXT("\'")) {
			Info.Name = TEXT("KCS.SyntaxHighlight.Normal");
			TextStyle = SyntaxTextStyle.StringTextStyle;
			ParserState = EParserState::None;
		} else if (ParserState==EParserState::None && Keyword==TEXT("=>")) {
			Info.Name = TEXT("KCS.SyntaxHighlight.Macro");
			TextStyle = SyntaxTextStyle.MacroTextStyle;
			ParserState = EParserState::None;
		} else if (!TChar<WIDECHAR>::IsAlpha(Keyword[0])) {
			uint32 FBracket = CountChars(TEXT('['),Source); uint32 BBracket = CountChars(TEXT(']'),Source);
			uint32 FParent = CountChars(TEXT('('),Source); uint32 BParent = CountChars(TEXT(')'),Source);
			uint32 FBraces = CountChars(TEXT('{'),Source); uint32 BBraces = CountChars(TEXT('}'),Source);
			//
			if (ParserState==EParserState::LookingForChar) {
				Info.Name = TEXT("KCS.SyntaxHighlight.String");
				TextStyle = SyntaxTextStyle.StringTextStyle;
			} else if (ParserState!=EParserState::None) {
				Info.Name = TEXT("KCS.SyntaxHighlight.Processor");
				TextStyle = SyntaxTextStyle.ProcessorTextStyle;
			} else if ((Keyword[0]==TEXT('(')||Keyword[0]==TEXT(')'))&&(FParent!=BParent)) {
				Info.Name = TEXT("KCS.SyntaxHighlight.Illegal");
				TextStyle = SyntaxTextStyle.IllegalTextStyle;
			} else if ((Keyword[0]==TEXT('[')||Keyword[0]==TEXT(']'))&&(FBracket!=BBracket)) {
				Info.Name = TEXT("KCS.SyntaxHighlight.Illegal");
				TextStyle = SyntaxTextStyle.IllegalTextStyle;
			} else if ((Keyword[0]==TEXT('{')||Keyword[0]==TEXT('}'))&&(FBraces!=BBraces)) {
				Info.Name = TEXT("KCS.SyntaxHighlight.Illegal");
				TextStyle = SyntaxTextStyle.IllegalTextStyle;
			} else {
				Info.Name = TEXT("KCS.SyntaxHighlight.Operator");
				TextStyle = SyntaxTextStyle.OperatorTextStyle;
			}///
		}///
	} else if (ParserState==EParserState::None && (Keyword.IsNumeric()||Keyword.Replace(TEXT("f"),TEXT("")).IsNumeric()||Keyword.Replace(TEXT(".f"),TEXT("")).IsNumeric())) {
		Info.Name = TEXT("KCS.SyntaxHighlight.Number");
		TextStyle = SyntaxTextStyle.NumberTextStyle;
	} else if (ParserState==EParserState::None) {
		if (Keyword==TEXT("/")) {
			Info.Name = TEXT("KCS.SyntaxHighlight.Operator");
			TextStyle = SyntaxTextStyle.OperatorTextStyle;
		} else if (Keyword==TEXT("*")) {
			Info.Name = TEXT("KCS.SyntaxHighlight.Operator");
			TextStyle = SyntaxTextStyle.OperatorTextStyle;
		}///
		//
		for (const auto &K : CS::Keywords) {
			if (FString(K).Equals(Keyword,ESearchCase::CaseSensitive)) {
				Info.Name = TEXT("KCS.SyntaxHighlight.Keyword");
				TextStyle = SyntaxTextStyle.KeywordTextStyle;
			}///
		}///
		//
		for (const auto &M : CS::Macros) {
			if (FString(M).Equals(Keyword,ESearchCase::CaseSensitive)) {
				Info.Name = TEXT("KCS.SyntaxHighlight.Macro");
				TextStyle = SyntaxTextStyle.MacroTextStyle;
			}///
		}///
		//
		for (const auto &T : CS::Types) {
			if (FString(T).Equals(Keyword,ESearchCase::CaseSensitive)) {
				Info.Name = TEXT("KCS.SyntaxHighlight.Type");
				TextStyle = SyntaxTextStyle.TypeTextStyle;
			}///
		}///
		//
		for (const auto &C : CS::Containers) {
			if (FString(C).Equals(Keyword,ESearchCase::CaseSensitive)) {
				Info.Name = TEXT("KCS.SyntaxHighlight.Container");
				TextStyle = SyntaxTextStyle.ContainerTextStyle;
			}///
		}///
		//
		if (ScriptSource.IsValid()) {
			if (IsNamespace(Keyword)) {
				Info.Name = TEXT("KCS.SyntaxHighlight.Event");
				TextStyle = SyntaxTextStyle.EventTextStyle;
				return;
			} else if (IsNamespace(Context+Keyword)) {
				Info.Name = TEXT("KCS.SyntaxHighlight.Event");
				TextStyle = SyntaxTextStyle.EventTextStyle;
				return;
			}///
			//
			if (IsClassType(Keyword)) {
				Info.Name = TEXT("KCS.SyntaxHighlight.Class");
				TextStyle = SyntaxTextStyle.ClassTextStyle;
				return;
			} else if (IsClassType(Context+Keyword)) {
				Info.Name = TEXT("KCS.SyntaxHighlight.Class");
				TextStyle = SyntaxTextStyle.ClassTextStyle;
				return;
			}///
			//
			if (IsFieldType(Keyword)) {
				Info.Name = TEXT("KCS.SyntaxHighlight.Property");
				TextStyle = SyntaxTextStyle.PropertyTextStyle;
				return;
			} else if (IsFieldType(Context+Keyword)) {
				Info.Name = TEXT("KCS.SyntaxHighlight.Property");
				TextStyle = SyntaxTextStyle.PropertyTextStyle;
				return;
			}///
			//
			if (IsPropertyType(Keyword)) {
				Info.Name = TEXT("KCS.SyntaxHighlight.Property");
				TextStyle = SyntaxTextStyle.PropertyTextStyle;
				return;
			} else if (IsPropertyType(Context+Keyword)) {
				Info.Name = TEXT("KCS.SyntaxHighlight.Property");
				TextStyle = SyntaxTextStyle.PropertyTextStyle;
				return;
			}///
			//
			if (IsFunctionType(Keyword)) {
				Info.Name = TEXT("KCS.SyntaxHighlight.Function");
				TextStyle = SyntaxTextStyle.FunctionTextStyle;
				return;
			} else if (IsFunctionType(Context+Keyword)) {
				Info.Name = TEXT("KCS.SyntaxHighlight.Function");
				TextStyle = SyntaxTextStyle.FunctionTextStyle;
				return;
			}///
		}///
	}///
	//
	if (Token.Type==FSyntaxTokenizer::ETokenType::Literal) {
		if (ParserState==EParserState::LookingForSingleLineComment) {
			Info.Name = TEXT("KCS.SyntaxHighlight.Comment");
			TextStyle = SyntaxTextStyle.CommentTextStyle;
		} else if (ParserState==EParserState::LookingForMultiLineComment) {
			Info.Name = TEXT("KCS.SyntaxHighlight.Comment");
			TextStyle = SyntaxTextStyle.CommentTextStyle;
		} else if (ParserState==EParserState::LookingForString) {
			Info.Name = TEXT("KCS.SyntaxHighlight.String");
			TextStyle = SyntaxTextStyle.StringTextStyle;
		} else if (ParserState==EParserState::LookingForChar) {
			Info.Name = TEXT("KCS.SyntaxHighlight.String");
			TextStyle = SyntaxTextStyle.StringTextStyle;
		}///
	}///
}

bool FTextSyntaxHighlighter::IsNamespace(const FString &Word) const {
	if (IsAutoCompleteTree(Word)) {
		TArray<FString>Tree = ParseAutoCompleteTree(Word);
		const FMonoNamespaceDefinition &SpaceInfo = IKCS_MonoAnalyzer::GetNamespaceInfo(ScriptSource.Get(),Tree,TEXT(""));
		return SpaceInfo.IsValid();
	} else {
		const FMonoNamespaceDefinition &SpaceInfo = IKCS_MonoAnalyzer::GetNamespaceInfo(ScriptSource.Get(),Word);
		return SpaceInfo.IsValid();
	}///
	//
	return false;
}

bool FTextSyntaxHighlighter::IsClassType(const FString &Word) const {
	if (IsAutoCompleteTree(Word)) {
		TArray<FString>Tree{};
		//
		for (const auto &Item : ParseAutoCompleteTree(Word)) {
			Tree.Add(FilterKeyword(Item));
		}///
		//
		const FString Context = Tree.Pop(true);
		const FMonoClassDefinition &ClassInfo = IKCS_MonoAnalyzer::GetClassInfo(ScriptSource.Get(),Tree,Context);
		//
		return ClassInfo.IsValid();
	} else {
		const FMonoClassDefinition &ClassInfo = IKCS_MonoAnalyzer::GetClassInfo(ScriptSource.Get(),Word);
		return ClassInfo.IsValid();
	}///
	//
	return false;
}

bool FTextSyntaxHighlighter::IsFieldType(const FString &Word) const {
	if (IsAutoCompleteTree(Word)) {
		TArray<FString>Tree{};
		//
		for (const auto &Item : ParseAutoCompleteTree(Word)) {
			Tree.Add(FilterKeyword(Item));
		}///
		//
		const FString Context = Tree.Pop(true);
		const FMonoFieldDefinition &FieldInfo = IKCS_MonoAnalyzer::GetFieldInfo(ScriptSource.Get(),Tree,Context);
		//
		return FieldInfo.IsValid();
	} else {
		const FMonoFieldDefinition &FieldInfo = IKCS_MonoAnalyzer::GetFieldInfo(ScriptSource.Get(),Word);
		return FieldInfo.IsValid();
	}///
	//
	return false;
}

bool FTextSyntaxHighlighter::IsPropertyType(const FString &Word) const {
	if (IsAutoCompleteTree(Word)) {
		TArray<FString>Tree{};
		//
		for (const auto &Item : ParseAutoCompleteTree(Word)) {
			Tree.Add(FilterKeyword(Item));
		}///
		//
		const FString Context = Tree.Pop(true);
		const FMonoFieldDefinition &PropInfo = IKCS_MonoAnalyzer::GetPropertyInfo(ScriptSource.Get(),Tree,Context);
		//
		return PropInfo.IsValid();
	} else {
		const FMonoFieldDefinition &PropInfo = IKCS_MonoAnalyzer::GetPropertyInfo(ScriptSource.Get(),Word);
		return PropInfo.IsValid();
	}///
	//
	return false;
}

bool FTextSyntaxHighlighter::IsFunctionType(const FString &Word) const {
	if (IsAutoCompleteTree(Word)) {
		TArray<FString>Tree{};
		//
		for (const auto &Item : ParseAutoCompleteTree(Word)) {
			Tree.Add(FilterKeyword(Item));
		}///
		//
		const FString Context = Tree.Pop(true);
		const FMonoMethodDefinition &MethodInfo = IKCS_MonoAnalyzer::GetMethodInfo(ScriptSource.Get(),Tree,Context);
		//
		return MethodInfo.IsValid();
	} else {
		const FMonoMethodDefinition &MethodInfo = IKCS_MonoAnalyzer::GetMethodInfo(ScriptSource.Get(),Word);
		return MethodInfo.IsValid();
	}///
	//
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const TArray<FString> FTextSyntaxHighlighter::ParseAutoCompleteTree(const FString &Word) const {
	TArray<FString>Tree{};
	//
	if (!Word.Contains(TEXT("."))) {
		Tree.Add(Word);
	} else {
		Word.ParseIntoArray(Tree,TEXT("."));
	}///
	//
	return Tree;
}

const FString FTextSyntaxHighlighter::FilterKeyword(const FString &Keyword) const {
	FString Subject = Keyword;
	FString Raw{};
	//
	bool IsMethodCall = false;
	int32 Open = CountChars(TEXT(')'),Subject);
	//
	if (Open>0) {
		int32 Range = Subject.Len()-1;
		//
		for (int32 R=Range; R>=0; --R) {
			if (Subject[R]==TEXT('(')){--Open; continue;}
			if (Open<1) {Raw.AppendChar(Subject[R]); IsMethodCall=true;}
		}///
		//
		Raw.ReverseString();
		Subject = Raw.TrimStartAndEnd();
	} else if (Subject.Contains(TEXT("("))) {
		Subject.Split(TEXT("("),nullptr,&Subject,ESearchCase::IgnoreCase,ESearchDir::FromEnd);
	}///
	//
	Open = CountChars(TEXT('>'),Subject);
	//
	if (Open>0) {
		Raw.Empty();
		int32 Range = Subject.Len()-1;
		//
		for (int32 R=Range; R>=0; --R) {
			if (Subject[R]==TEXT('<')){--Open; continue;}
			if (Open<1) {Raw.AppendChar(Subject[R]);}
		}///
		//
		Raw.ReverseString();
		Subject = Raw.TrimStartAndEnd();
	} else if (Subject.Contains(TEXT("<"))) {
		Subject.Split(TEXT("<"),&Subject,nullptr,ESearchCase::IgnoreCase,ESearchDir::FromEnd);
	}///
	//
	if (IsMethodCall) {Subject.AppendChar(TEXT('@'));}
	//
	return Subject;
}

uint32 FTextSyntaxHighlighter::CountChars(const TCHAR &CH, const FString &Word) const {
	int32 Count = 0;
	//
	for (const TCHAR &CC : Word.GetCharArray()) {
		if (CC==CH) {Count++;}
	}///
	//
	return Count;
}

bool FTextSyntaxHighlighter::IsAutoComplete(const FString &Word) const {
	return (
		(Word.EndsWith(TEXT("."))||Word.EndsWith(TEXT("@"))) &&
		(!IsAutoCompleteTree(Word))
	);///
}

bool FTextSyntaxHighlighter::IsAutoCompleteTree(const FString &Word) const {
	int32 Count = 0;
	//
	for (const TCHAR &CH : Word.GetCharArray()) {
		if (CH==TEXT('.')) {Count++;}
	}///
	//
	return (
		((Word.EndsWith(TEXT("."))||Word.EndsWith(TEXT("@")))&&(Count>1)) ||
		(!Word.EndsWith(TEXT("."))&&(Count>0))
	);///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////