/**
 * Syncplicity, LLC © 2014 
 * 
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 * 
 * If you would like a copy of source code for this product, EMC will provide a
 * copy of the source code that is required to be made available in accordance
 * with the applicable open source license.  EMC may charge reasonable shipping
 * and handling charges for such distribution.  Please direct requests in writing
 * to EMC Legal, 176 South St., Hopkinton, MA 01748, ATTN: Open Source Program
 * Office.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along
 * with this library; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 */

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

using System;
using System.Collections.Generic;

using Liferay.Nativity.Control;

namespace Liferay.Nativity.Modules.FileIcon.Unix
{
	/**
	 * @author Dennis Ju, ported to C# by Andrew Rondeau. Support for icons added by Ivan Burlakov
	 */
	public abstract class UnixFileIconControlBaseImpl : FileIconControlBase
	{
		private const int MESSAGE_BUFFER_SIZE = 500;

		public UnixFileIconControlBaseImpl(
			NativityControl nativityControl,
			FileIconControlCallback fileIconControlCallback)
			: base(nativityControl, fileIconControlCallback)
		{
		}

		public override int RegisterIcon (string path)
		{
			var message = new NativityMessage (Constants.REGISTER_ICON, path);
			
			var reply = nativityControl.SendMessage (message);
			
			if (string.IsNullOrEmpty (reply))
			{
				return -1;
			}

			int replyInt;
			if (int.TryParse (reply, out replyInt))
			{
				return replyInt;
			}
			else
			{
				return -1;
			}
		}

        public override int RegisterMenuIcon (string path)
        {
            var message = new NativityMessage (Constants.REGISTER_MENU_ICON, path);
            
            var reply = nativityControl.SendMessage (message);
            
            if (string.IsNullOrEmpty (reply))
            {
                return -1;
            }
            
            int replyInt;
            if (int.TryParse (reply, out replyInt))
            {
                return replyInt;
            }
            else
            {
                return -1;
            }
        }
		
		public override void RemoveAllFileIcons()
		{
			var message = new NativityMessage(Constants.REMOVE_ALL_FILE_ICONS, string.Empty);
			this.nativityControl.SendMessage(message);
		}
		
		public override void UnregisterIcon(int id)
		{
			var message = new NativityMessage(Constants.UNREGISTER_ICON, id);
			this.nativityControl.SendMessage(message);
		}
	}}
