/**
 * Copyright (c) 2000-2013 Liferay, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */

#include "ContextMenuConstants.h"
#include "ContextMenuUtil.h"
#include "ContextMenuItem.h"
#include "FileUtil.h"
#include "ParserUtil.h"
#include "RegistryUtil.h"
#include "UtilConstants.h"
#include "json/json.h"

#include <vector>

using namespace std;

ContextMenuUtil::ContextMenuUtil() : _menuList(0)
{
	_communicationSocket = new CommunicationSocket(PORT);
	_selectedFiles = new vector<wstring>;
}

ContextMenuUtil::~ContextMenuUtil(void)
{
	if(_communicationSocket != 0)
	{
		delete _communicationSocket;
	}
		
	if(_menuList != 0)
	{
		delete _menuList;
	}

	_selectedFiles = 0;
}

bool ContextMenuUtil::AddFile(wstring file)
{
	_selectedFiles->push_back(file);

	return true;
}

bool ContextMenuUtil::GetContextMenuItem(int index, ContextMenuItem** item)
{
	return _GetContextMenuItem(index, _menuList, item);
}

bool ContextMenuUtil::_GetContextMenuItem(int index, vector<ContextMenuItem*>* menus, ContextMenuItem** item)
{
	for(vector<ContextMenuItem*>::iterator it = menus->begin(); it != menus->end(); it++)
	{
		ContextMenuItem* temp = *it;
		if(temp->GetIndex() == index)
		{
			*item = temp;
			return true;
		}

		if(temp->GetContextMenuItems() != 0 && temp->GetContextMenuItems()->size() > 0)
		{
			if(_GetContextMenuItem(index, temp->GetContextMenuItems(), item))
			{
				return true;
			}
		}
	}

	return false;
}

bool ContextMenuUtil::GetMenus(vector<ContextMenuItem*> *menuList)
{
	if(_menuList == 0)
	{
		return false;
	}

	menuList->insert(menuList->end(), _menuList->begin(), _menuList->end());

	return true;
}

bool ContextMenuUtil::IsMenuNeeded(void)
{  
	if(FileUtil::IsChildFileOfRoot(_selectedFiles))
	{
		return true;
	}

	return false;
}

bool ContextMenuUtil::InitMenus(void)
{
	_menuList = new vector<ContextMenuItem*>();

	Json::Value jsonRoot;

	jsonRoot[NATIVITY_COMMAND] = NATIVITY_GET_CONTEXT_MENU_LIST;

	for (vector<wstring>::iterator it = _selectedFiles->begin(); it != _selectedFiles->end(); it++)
	{
		jsonRoot[NATIVITY_VALUE].append(ParserUtil::toString(*it));
	}

	Json::FastWriter jsonWriter;

	wstring* getMenuMessage = new wstring();
	
	getMenuMessage->append(ParserUtil::toWstring(jsonWriter.write(jsonRoot)));

	bool success = true;

	wstring* getMenuReceived = new wstring();

	if(_communicationSocket->SendMessageReceiveResponse(getMenuMessage->c_str(), getMenuReceived))
	{
		Json::Value jsonContextMenuItemsList = ParserUtil::GetJsonValue(NATIVITY_VALUE, getMenuReceived);

		for (unsigned int i = 0; i < jsonContextMenuItemsList.size(); i++)
		{
			Json::Value jsonContextMenuItem = jsonContextMenuItemsList[i];

			ContextMenuItem* contextMenuItem = new ContextMenuItem();

			if (!_ParseContextMenuItem(jsonContextMenuItem, contextMenuItem))
			{
				success = false;

				break;
			}

			_menuList->push_back(contextMenuItem);
		}
	}

	delete getMenuReceived;
	delete getMenuMessage;

	return success;
}

bool ContextMenuUtil::_ParseContextMenuItem(const Json::Value &jsonContextMenuItem, ContextMenuItem* contextMenuItem)
{
	// enabled
	bool enabled = jsonContextMenuItem.get(NATIVITY_ENABLED, true).asBool();

	contextMenuItem->SetEnabled(enabled);

	// title
	wstring *title = new wstring();

	title->append(ParserUtil::toWstring(jsonContextMenuItem.get(NATIVITY_TITLE, "").asString()));

	if (title->size() == 0)
	{
		return false;
	}

	contextMenuItem->SetTitle(title);

	// uuid

	wstring *uuid = new wstring();
	
	uuid->append(ParserUtil::toWstring(jsonContextMenuItem.get(NATIVITY_UUID, "").asString()));

	if (uuid->size() == 0)
	{
		return false;
	}

	contextMenuItem->SetUuid(uuid);

	// help text

	wstring* helpText = new wstring();
	
	helpText->append(ParserUtil::toWstring(jsonContextMenuItem.get(NATIVITY_HELP_TEXT, "").asString()));

	contextMenuItem->SetHelpText(helpText);

	// children context menu items

	Json::Value jsonChildrenContextMenuItems = jsonContextMenuItem.get(NATIVITY_CONTEXT_MENU_ITEMS, "");

	vector<ContextMenuItem*>* childrenContextMenuItems = new vector<ContextMenuItem*>();

	for (unsigned int i = 0; i < jsonChildrenContextMenuItems.size(); i++)
	{
		Json::Value jsonChildContextMenuItem = jsonChildrenContextMenuItems[i];

		ContextMenuItem* childContextMenuItem = new ContextMenuItem();

		_ParseContextMenuItem(jsonChildContextMenuItem, childContextMenuItem);

		childrenContextMenuItems->push_back(childContextMenuItem);
	}

	contextMenuItem->SetContextMenuItems(childrenContextMenuItems);

	return true;
}

bool ContextMenuUtil::GetContextMenuAction(std::wstring* title, ContextMenuAction** item)
{
	for(vector<ContextMenuItem*>::iterator it = _menuList->begin(); it != _menuList->end(); it++)
	{
		ContextMenuItem* temp = *it;
		wstring* currentTitle = temp->GetTitle();

		if(currentTitle->compare(*title) == 0)
		{
			ContextMenuAction* action = new ContextMenuAction();
			action->SetUuid(temp->GetUuid());
			action->SetFiles(_selectedFiles);

			item = &action;
			return true;
		}
	}

	return false;
}

bool ContextMenuUtil::GetContextMenuAction(int action, ContextMenuAction** item)
{
	ContextMenuItem* contextMenuItem;

	if(GetContextMenuItem(action, &contextMenuItem))
	{
		ContextMenuAction* action = new ContextMenuAction();
		action->SetUuid(contextMenuItem->GetUuid());
		action->SetFiles(_selectedFiles);

		item = &action;
			
		return true;
	}

	return false;
}

bool ContextMenuUtil::PerformAction(int command)
{
	ContextMenuItem* contextMenuItem;
	
	if(!GetContextMenuItem(command, &contextMenuItem))
	{
		return false;
	}

	Json::Value jsonValue;

	jsonValue[NATIVITY_UUID] = ParserUtil::toString(contextMenuItem->GetUuid()->c_str());

	for (vector<wstring>::iterator it = _selectedFiles->begin(); it != _selectedFiles->end(); it++)
	{
		wstring selectedFile = *it;

		jsonValue[NATIVITY_FILES].append(ParserUtil::toString(selectedFile));
	}

	Json::Value jsonRoot;

	jsonRoot[NATIVITY_COMMAND] = NATIVITY_CONTEXT_MENU_ACTION;
	jsonRoot[NATIVITY_VALUE] = jsonValue;

	Json::FastWriter jsonWriter;

	wstring* jsonMessage = new wstring();

	jsonMessage->append(ParserUtil::toWstring(jsonWriter.write(jsonRoot)));
	
	wstring* response = new wstring();

	if(!_communicationSocket->SendMessageReceiveResponse(jsonMessage->c_str(), response))
	{
		return false;
	}

	return true;
}