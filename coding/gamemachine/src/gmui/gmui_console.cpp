﻿#include "stdafx.h"
#include "gmui_console.h"
#if _WINDOWS
#	include "gmui_console_ui.h"
#endif

GMUIConsole::~GMUIConsole()
{
	if (GMDebugger::getDebugOutput() == this)
		GMDebugger::setDebugOutput(nullptr);
}

void GMUIConsole::newConsoleWindow(OUT GMUIConsole** out)
{
	ASSERT(*out);
	*out = new GMUIConsole();
}

GMUIStringPtr GMUIConsole::getWindowClassName() const
{
	return _L("gamemachine_Console_class");
}

#if _WINDOWS
template <typename T>
inline static T* findControl(GMUIPainter* painter, GMWchar* name)
{
	T* control = static_cast<T*>(painter->FindControl(name));
	ASSERT(control);
	return control;
}

LongResult GMUIConsole::onCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	D(d);
	d->painter->Init(getWindowHandle());
	DuiLib::CDialogBuilder builder;
	//CDialogBuilderCallbackEx cb;
	DuiLib::CControlUI* pRoot = builder.Create(gmui::GMUIConsole::UI, (UINT)0, NULL, d->painter);
	ASSERT(pRoot && "Failed to parse XML");
	d->painter->AttachDialog(pRoot);
	d->painter->AddNotifier(this);
	LongResult result = Base::onCreate(uMsg, wParam, lParam, bHandled);

	afterCreated();
	bHandled = TRUE;
	return result;
}

LongResult GMUIConsole::onClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	hideWindow();
	bHandled = TRUE;
	return 0;
}

LongResult GMUIConsole::onShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	D(d);
	while (!d->msgQueue.empty())
	{
		Data::Message& str = d->msgQueue.front();
		insertTextToRichEdit(str.type, str.message);
		d->msgQueue.pop();
	}
	return 0;
}

void GMUIConsole::onFinalMessage(GMUIWindowHandle wndHandle)
{
	delete this;
}

void GMUIConsole::Notify(DuiLib::TNotifyUI& msg)
{
	D(d);
	if (msg.sType == DUI_MSGTYPE_CLICK)
	{
		if (msg.pSender == d->optLog)
			selectTab(0);
		else if (msg.pSender == d->optPerformance)
			selectTab(1);
		else
		{
			Data::OutputType type;
			if (msg.pSender == d->optFltInfo)
				type = Data::Info;
			else if (msg.pSender == d->optFltWarning)
				type = Data::Warning;
			else if (msg.pSender == d->optFltError)
				type = Data::Error;
			else if (msg.pSender == d->optFltDebug)
				type = Data::Debug;

			d->filter.toggle(type);
			onFilterChanged();
		}
	}
	else if (msg.sType == DUI_MSGTYPE_WINDOWINIT)
	{
		refreshTabs();
		refreshOptFilter();
	}
}

void GMUIConsole::afterCreated()
{
	D(d);
	DuiLib::CRichEditUI* re = static_cast<DuiLib::CRichEditUI*> (d->painter->FindControl(ID_EDIT_CONSOLE));
	ASSERT(re);	
	d->consoleEdit = re;
	d->consoleEdit->SetBkColor(0);

	d->tabLayout = findControl<DuiLib::CTabLayoutUI>(d->painter, ID_TABLAYOUT);
	d->optLog = findControl<DuiLib::COptionUI>(d->painter, ID_OPTION_LOG);
	d->optPerformance = findControl<DuiLib::COptionUI>(d->painter, ID_OPTION_PERFORMACE);
	d->optFltInfo = findControl<DuiLib::COptionUI>(d->painter, ID_OPTION_FILTER_INFO);
	d->optFltWarning = findControl<DuiLib::COptionUI>(d->painter, ID_OPTION_FILTER_WARNING);
	d->optFltError = findControl<DuiLib::COptionUI>(d->painter, ID_OPTION_FILTER_ERROR);
	d->optFltDebug = findControl<DuiLib::COptionUI>(d->painter, ID_OPTION_FILTER_DEBUG);
}

void GMUIConsole::selectTab(GMint i)
{
	D(d);
	d->tabIndex = i;
	d->tabLayout->SelectItem(i);
	refreshTabs();
}

void GMUIConsole::addBuffer(Data::OutputType type, const GMString& msg)
{
	D(d);
	const static GMuint MAX_BUFFER = 1000;
	d->msgBuffer.push_back ({ type, msg });
	if (d->msgBuffer.size() > MAX_BUFFER)
		d->msgBuffer.pop_front();
}

void GMUIConsole::onFilterChanged()
{
	D(d);
	refreshOptFilter();

	d->consoleEdit->SetText(_L(""));
	for (auto& msg : d->msgBuffer)
	{
		insertText(msg.type, msg.message);
	}
}

void GMUIConsole::refreshTabs()
{
	D(d);
	GMint i = 0;
	auto tabOptions = { d->optLog, d->optPerformance };
	for (auto opt : tabOptions)
	{
		if (i == d->tabIndex)
		{
			opt->SetBorderSize(1);
			opt->SetBorderColor(0xFFFFFF);
		}
		else
		{
			opt->SetBorderSize(1);
			opt->SetBorderColor(0x000000);
		}
		i++;
	}
}

void GMUIConsole::refreshOptFilter()
{
	D(d);
	auto typeList =  { Data::Info,		Data::Warning,		Data::Error,		Data::Debug };
	auto uiList   =  { d->optFltInfo,	d->optFltWarning,	d->optFltError,		d->optFltDebug };
	
	auto iter = uiList.begin();
	for (auto type : typeList)
	{
		DuiLib::COptionUI* opt = const_cast<DuiLib::COptionUI*>(*iter);
		if (d->filter.isSet(type))
		{
			opt->SetBorderSize(1);
			opt->SetBorderColor(0xFFFFFF);
		}
		else
		{
			opt->SetBorderSize(1);
			opt->SetBorderColor(0x000000);
		}
		iter++;
	}
}

void GMUIConsole::info(const GMString& msg)
{
	D(d);
	addBuffer(Data::Info, msg);
	insertText(Data::Info, msg);
}

void GMUIConsole::warning(const GMString& msg)
{
	D(d);
	addBuffer(Data::Warning, msg);
	insertText(Data::Warning, msg);
}

void GMUIConsole::error(const GMString& msg)
{
	D(d);
	addBuffer(Data::Error, msg);
	insertText(Data::Error, msg);
}

void GMUIConsole::debug(const GMString& msg)
{
	D(d);
	addBuffer(Data::Debug, msg);
	insertText(Data::Debug, msg);
}

void GMUIConsole::insertText(Data::OutputType type, const GMString& msg)
{
	D(d);
	if (isWindowVisible())
	{
		if (d->filter.isSet(type))
			insertTextToRichEdit(type, msg);
	}
	else
	{
		d->msgQueue.push({ type, msg });
	}
}

void GMUIConsole::insertTextToRichEdit(Data::OutputType type, const GMString& msg)
{
	D(d);
	DWORD color;
	switch (type)
	{
	case GMUIConsolePrivate::Info:
		color = 0x555555;
		break;
	case GMUIConsolePrivate::Warning:
		color = 0x00FFFF;
		break;
	case GMUIConsolePrivate::Error:
		color = 0x3300CC;
		break;
	case GMUIConsolePrivate::Debug:
		color = 0xFF0099;
		break;
	default:
		break;
	}

	CHARRANGE cr1, cr2;
	d->consoleEdit->GetSel(cr1);
	d->consoleEdit->InsertText(0, (msg + _L("\n")).toStdWString().c_str());
	d->consoleEdit->EndRight();
	d->consoleEdit->GetSel(cr2);
	d->consoleEdit->SetSel(cr1.cpMin, cr2.cpMax);
	CHARFORMAT2 cf;
	d->consoleEdit->GetSelectionCharFormat(cf);
	cf.crTextColor = color;
	d->consoleEdit->SetSelectionCharFormat(cf);
}

#endif