/*
  KeePerso - A Plugin for german Personalausweis
  Copyright (C) 2021 buergerservice.org e.V. <KeePerso@buergerservice.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once


using namespace System;
using namespace System::Windows::Forms;
using namespace KeePass::Plugins;
using namespace KeePassLib::Keys;

// The namespace name must be the same as the file name of the
// plugin without its extension.
// For example, if you compile a plugin 'SamplePluginCpp.dll',
// the namespace must be named 'SamplePluginCpp'.
namespace KeePersoCpp
{

	public ref class PersoKeyProvider : KeyProvider
	{
	public:
		property System::String^ Name {
			System::String^ get() override
			{
				return "KeePerso KeyProvider";
			}
		}

		array<System::Byte>^ GetKey(KeyProviderQueryContext^ ctx) override;
		void sethwnd(HWND hWndMain);
		HWND gethwnd();

		property array<System::Byte>^ provider_key;
		property HWND hwndvar;
		void runworkflow();
	};


	// Namespace name 'SamplePluginCpp' + 'Ext' = 'SamplePluginCppExt'
	public ref class KeePersoCppExt : Plugin
	{
	public:
		virtual bool Initialize(IPluginHost^ host) override;
		virtual void Terminate() override;
		virtual ToolStripMenuItem^ GetMenuItem(PluginMenuType t) override;
		
	private:
		void OnMenuOpening(Object^ sender, EventArgs^ e);
		void OnMenuKeePerso(Object^ sender, EventArgs^ e);
		void OnMenuShowCertificate(Object^ sender, EventArgs^ e);

		// The plugin remembers its host in this variable
		IPluginHost^ m_host;
		PersoKeyProvider ^ m_prov = gcnew PersoKeyProvider();

	};

	
}
