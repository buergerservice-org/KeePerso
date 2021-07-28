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


#include "StdAfx.h"
#include <AtlBase.h>
#include <atlconv.h>
#include "KeePersoCpp.h"

#include "workflow.h"
#include "SG_InputBoxLib.h"
#include <msclr/marshal_cppstd.h>
#include <time.h>
#include <locale>

//#include "json/json.h" //jsoncpp uses MIT-Licence

using namespace KeePass;
using namespace KeePassLib;
using namespace KeePassLib::Security;
using namespace KeePassLib::Utility;


#define MIID_ADDENTRIES _T("KeePersoCpp_AddEntries")

workflow w;


namespace KeePersoCpp {


	int StringToWString(std::wstring& ws, const std::string& s)
	{
		std::wstring wsTmp(s.begin(), s.end());
		
		ws = wsTmp;
		
		return 0;
	}

	bool LPWToString(std::string& s, const LPWSTR pw, UINT codepage = CP_ACP)
	{
		bool res = false;
		char* p = 0;
		int bsz;

		bsz = WideCharToMultiByte(codepage, 0, pw, -1, 0, 0, 0, 0);
		if (bsz > 0) {
			p = new char[bsz];
			int rc = WideCharToMultiByte(codepage, 0, pw, -1, p, bsz, 0, 0);
			if (rc != 0) {
				p[bsz - 1] = 0;
				s = p;
				res = true;
			}
		}
		delete[] p;
		return res;
	}

	void findAndReplaceAll(std::string& data, std::string toSearch, std::string replaceStr)
	{
		// Get the first occurrence
		size_t pos = data.find(toSearch);
		// Repeat till end is reached
		while (pos != std::string::npos)
		{
			// Replace this occurrence of Sub String
			data.replace(pos, toSearch.size(), replaceStr);
			// Get the next occurrence from the current position
			pos = data.find(toSearch, pos + replaceStr.size());
		}
	}

	void wfindAndReplaceAll(std::wstring& data, std::wstring toSearch, std::wstring replaceStr)
	{
		// Get the first occurrence
		size_t pos = data.find(toSearch);
		// Repeat till end is reached
		while (pos != std::string::npos)
		{
			// Replace this occurrence of Sub String
			data.replace(pos, toSearch.size(), replaceStr);
			// Get the next occurrence from the current position
			pos = data.find(toSearch, pos + replaceStr.size());
		}
	}


	DWORD WINAPI  CreateNoteMessageBox(LPVOID lpParam) {
		::MessageBox(NULL, (LPCTSTR)lpParam, _T("KeePersoNote"), MB_OK);
		return 0;
	}

	void CloseNoteMessageBox()
	{
		//find Informationwindow in thread and close
		HWND hWndNote = ::FindWindow(NULL, _T("KeePersoNote"));
		if (hWndNote)
		{
			::PostMessage(hWndNote, WM_CLOSE, 0, 0);
		}
	}

	bool KeePersoCppExt::Initialize(IPluginHost^ host)
	{
		if(host == nullptr) return false; // Fail; we need the host
		m_host = host;

		srand(GetTickCount());
		
		m_host->KeyProviderPool->Add(m_prov);
		
		HWND hWndMain = reinterpret_cast<HWND>(m_host->MainWindow->Handle.ToPointer());
		m_prov->sethwnd(hWndMain);
		m_prov->provider_key = gcnew array<Byte>(0);
		std::locale::global(std::locale("German_germany.UTF-8"));
		return true; // Initialization successful
	}


	void KeePersoCppExt::Terminate()
	{
		// Here the plugin should free all resources, close files/streams,
		// remove event handlers, etc.
		// In SamplePluginCpp, there's nothing to clean up.
		m_host->KeyProviderPool->Remove(m_prov);
	}


	ToolStripMenuItem^ KeePersoCppExt::GetMenuItem(PluginMenuType t)
	{
		// Our menu item below is intended for the main location(s),
		// not for other locations like the group or entry menus
		if(t != PluginMenuType::Main) return nullptr;

		ToolStripMenuItem^ tsmi = gcnew ToolStripMenuItem(_T("KeePerso"));

		ToolStripMenuItem^ tsmiCertificate = gcnew ToolStripMenuItem();
		tsmiCertificate->Text = _T("Show Certificate");
		//tsmiCertificate->Name = MIID_ADDENTRIES;
		tsmiCertificate->Click += gcnew EventHandler(this,
			&KeePersoCppExt::OnMenuShowCertificate);
		tsmi->DropDownItems->Add(tsmiCertificate);

		tsmi->DropDownItems->Add(gcnew ToolStripSeparator());

		ToolStripMenuItem^ tsmiKeePerso = gcnew ToolStripMenuItem();
		tsmiKeePerso->Text = _T("Produce Key for KeePass2 with Online identification AusweisApp2");
		tsmiKeePerso->Click += gcnew EventHandler(this,
			&KeePersoCppExt::OnMenuKeePerso);
		tsmi->DropDownItems->Add(tsmiKeePerso);

		// In our handler for the DropDownOpening event, we update the
		// states of our menu items (like disabling the 'Add Some Entries'
		// command when no database is open)
		tsmi->DropDownOpening += gcnew EventHandler(this,
			&KeePersoCppExt::OnMenuOpening);

		return tsmi;
	}


	void KeePersoCppExt::OnMenuOpening(Object^ sender, EventArgs^ e)
	{
		UNREFERENCED_PARAMETER(e);

		ToolStripMenuItem^ tsmi = dynamic_cast<ToolStripMenuItem^>(sender);
		if(tsmi == nullptr) { _ASSERT(false); return; }

		array<ToolStripItem^>^ a = tsmi->DropDownItems->Find(MIID_ADDENTRIES, false);
		if((a == nullptr) || (a->Length != 1)) { _ASSERT(false); return; }
		ToolStripItem^ tsiAddEntries = a[0];
		if(tsiAddEntries == nullptr) { _ASSERT(false); return; }

		PwDatabase^ pd = m_host->Database;
		bool bOpen = ((pd != nullptr) && pd->IsOpen);
		tsiAddEntries->Enabled = bOpen;
	}


	void KeePersoCppExt::OnMenuKeePerso(Object^ sender, EventArgs^ e)
	{
		UNREFERENCED_PARAMETER(sender);
		UNREFERENCED_PARAMETER(e);
		std::basic_string<TCHAR> strText;
		// Specify the main window as parent for the message box
		HWND hWndMain = reinterpret_cast<HWND>(m_host->MainWindow->Handle.ToPointer());

		if (m_prov->provider_key->Length != 0)
		{
			strText = _T("ERROR - key is already existing. Exiting Plugin.");
			// Display a native Win32 message box
			::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
				MB_ICONERROR | MB_OK);
			return;
		}

		m_prov->runworkflow();

	}


	void KeePersoCppExt::OnMenuShowCertificate(Object^ sender, EventArgs^ e)
	{
		//UNREFERENCED_PARAMETER(sender);
		//UNREFERENCED_PARAMETER(e);
		std::string outputstring = "";
		std::basic_string<TCHAR> strText;
		// Specify the main window as parent for the message box
		HWND hWndMain = reinterpret_cast<HWND>(m_host->MainWindow->Handle.ToPointer());
		std::locale::global(std::locale("German_germany.UTF-8"));

		outputstring=w.getcertificate();
		
		if (outputstring == "e1" || outputstring == "e3")
		{
			strText = _T("ERROR - please check AusweisApp2, cardreader and Personalausweis! Exiting Plugin.");
			// Display a native Win32 message box
			::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
				MB_ICONERROR | MB_OK);
			return;
		}
		//strText = _T("ERROR - key is already existing. Exiting Plugin.");
		//information for §18 (4) PAuswG
		std::string ap = "information about the certificate you are using to identify at provider:\n\n";
		ap.append("description: \n");
		ap.append("   issuerName: "+ w.issuerName +"\n");
		ap.append("   issuerUrl: " + w.issuerUrl + "\n");
		ap.append("   purpose: " + w.purpose + "\n");
		ap.append("   subjectName: " + w.subjectName + "\n");
		ap.append("   subjectUrl: " + w.subjectUrl + "\n\n");
		ap.append("termsOfUsage:\n" + w.termsOfUsage + "\n\n");
		ap.append("validity: \n");
		ap.append("   effectiveDate: " + w.effectiveDate + "\n");
		ap.append("   expirationDate: " + w.expirationDate + "\n");
		
		//StringToWString(strText, ap);
		CA2W ca2w(ap.c_str(), CP_UTF8);
		strText = ca2w;

		// Display a native Win32 message box
		::MessageBox(hWndMain, strText.c_str(), _T("KeePerso", lpCaption:LPCWSTR; uType:UINT),
			MB_ICONINFORMATION | MB_OK);
			return;

	}


	void PersoKeyProvider::runworkflow()
	{
		//workflow w;
		std::string outputstring = "";
		std::string PIN = "123456";
		int msgboxreturnvalue;
		// STL can be used
		std::basic_string<TCHAR> strText = _T("a service by buergerservice.org e.V.");
		
		HWND hWndMain = PersoKeyProvider::hwndvar;
		std::locale::global(std::locale("German_germany.UTF-8"));

		outputstring = w.getkeypad();

		//StringToWString(strText, outputstring);
		//msgboxreturnvalue= ::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
		//	MB_ICONQUESTION | MB_YESNO);

		if (outputstring == "e1")
		{
			strText = _T("ERROR - please check AusweisApp2, cardreader and Personalausweis! Exiting Plugin.");
			// Display a native Win32 message box
			::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
				MB_ICONERROR | MB_OK);
			return;
		}
		else if (outputstring == "e2")
		{
			strText = _T("ERROR - please check your Personalausweis! Exiting Plugin.");
			// Display a native Win32 message box
			::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
				MB_ICONINFORMATION | MB_OK);
			return;
		}
		else if (outputstring == "e3")
		{
			strText = _T("ERROR - please check your cardreader! Exiting Plugin.");
			// Display a native Win32 message box
			::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
				MB_ICONERROR | MB_OK);
			return;
		}
		else if (outputstring == "e4")
		{
			strText = _T("ERROR - AusweisApp2-version less than 1.22.* please update! Exiting Plugin.");
			// Display a native Win32 message box
			::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
				MB_ICONERROR | MB_OK);
			return;
		}
		else if (outputstring == "e5")
		{
			CloseNoteMessageBox();
			strText = _T("Warning - retryCounter of Perso <3, please start a selfauthentication direct with AusweisApp2! Exiting Plugin.");
			// Display a native Win32 message box
			::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
				MB_ICONWARNING | MB_OK);
			return;
		}
		else if (outputstring == "e7")
		{
			CloseNoteMessageBox();
			strText = _T("Error - found no cardreader! Exiting Plugin.");
			// Display a native Win32 message box
			::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
				MB_ICONERROR | MB_OK);
			return;
		}

		if (outputstring == "false")
		{
			LPWSTR result = SG_InputBox::GetString(
				L"KeePerso",
				L"Please enter PIN and Confirm this statement:\n Yes i want a key produced from my data.",
				L"");
			LPWToString(PIN, result);

			if (PIN == "")
			{
				strText = _T("ERROR - PIN is empty. Exiting Plugin.");
				// Display a native Win32 message box
				::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
					MB_ICONERROR | MB_OK);
				return;
			}

			if (PIN.length() < 6)
			{
				strText = _T("ERROR - PIN is too short. Exiting Plugin.");
				// Display a native Win32 message box
				::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
					MB_ICONERROR | MB_OK);
				return;
			}

			if (PIN.length() > 6)
			{
				strText = _T("ERROR - PIN is too long. Exiting Plugin.");
				// Display a native Win32 message box
				::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
					MB_ICONERROR | MB_OK);
				return;
			}
		}
		else
		{
			strText = _T("Please Confirm this statement:\n Yes i want a key produced from my data.\n(enter your PIN later in your cardreaderkeypad).");
			msgboxreturnvalue = ::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
				MB_ICONQUESTION | MB_YESNO);

			if (msgboxreturnvalue != 6)
			{
				strText = _T("Not confirmed. Exiting Plugin.");
				// Display a native Win32 message box
				::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
					MB_ICONWARNING | MB_OK);
				return;
			}

			PIN = "123456";
		}

		//open Informationwindow in thread
		CreateThread(NULL, 0, &CreateNoteMessageBox, _T("selfauthentication is running, please wait...\n(this window closes self-acting)"), 0, NULL);

		outputstring = w.startworkflow(PIN);

		//StringToWString(strText, outputstring);
		// Display a native Win32 message box
		//::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
		//	MB_ICONINFORMATION | MB_OK);

		PIN = "123456";
		if (outputstring == "e1")
		{
			CloseNoteMessageBox();
			strText = _T("ERROR - please check AusweisApp2, cardreader and Personalausweis! Exiting Plugin.");
			// Display a native Win32 message box
			::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
				MB_ICONERROR | MB_OK);
			return;
		}
		else if (outputstring == "e2")
		{
			CloseNoteMessageBox();
			strText = _T("ERROR - please check your Personalausweis! Exiting Plugin.");
			// Display a native Win32 message box
			::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
				MB_ICONERROR | MB_OK);
			return;
		}
		else if (outputstring == "e3")
		{
			CloseNoteMessageBox();
			strText = _T("ERROR - please check your cardreader! Exiting Plugin.");
			// Display a native Win32 message box
			::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
				MB_ICONERROR | MB_OK);
			return;
		}
		else if (outputstring == "e4")
		{
			CloseNoteMessageBox();
			strText = _T("ERROR - AusweisApp2-version less than 1.22.* please update! Exiting Plugin.");
			// Display a native Win32 message box
			::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
				MB_ICONERROR | MB_OK);
			return;
		}
		else if (outputstring == "e5")
		{
			CloseNoteMessageBox();
			strText = _T("Warning - retryCounter of Perso <3, please start a selfauthentication direct with AusweisApp2! Exiting Plugin.");
			// Display a native Win32 message box
			::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
				MB_ICONWARNING | MB_OK);
			return;
		}
		else if (outputstring == "e7")
		{
			CloseNoteMessageBox();
			strText = _T("Error - found no cardreader! Exiting Plugin.");
			// Display a native Win32 message box
			::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
				MB_ICONERROR | MB_OK);
			return;
		}
		else if (outputstring == "" || outputstring.length()<20)
		{
			CloseNoteMessageBox();
			strText = _T("ERROR - workflow was not successful! Exiting Plugin.");
			// Display a native Win32 message box
			::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
				MB_ICONERROR | MB_OK);
			return;
		}

		CloseNoteMessageBox();

		array<Byte>^ data = gcnew array<Byte>(outputstring.size());
		System::Runtime::InteropServices::Marshal::Copy(IntPtr(&outputstring[0]), data, 0, outputstring.size());
		provider_key = data;

		outputstring = w.GivenNames;
		outputstring = outputstring.append(", your KeePerso-Key is ready.\n\n\n");
	
		outputstring = outputstring.append("PersonalData read (for your information):\n\n");
		outputstring = outputstring.append("   AcademicTitle: " + w.AcademicTitle +"\n");
		outputstring = outputstring.append("   ArtisticName: " + w.ArtisticName + "\n");
		outputstring = outputstring.append("   BirthName: " + w.BirthName + "\n");
		outputstring = outputstring.append("   DateOfBirth: " + w.DateOfBirth + "\n");
		outputstring = outputstring.append("   DocumentType: " + w.DocumentType + "\n");
		outputstring = outputstring.append("   FamilyNames: " + w.FamilyNames + "\n");
		outputstring = outputstring.append("   GivenNames: " + w.GivenNames + "\n");
		outputstring = outputstring.append("   IssuingState: " + w.IssuingState + "\n");
		outputstring = outputstring.append("   Nationality: " + w.Nationality + "\n");
		outputstring = outputstring.append("      PlaceOfBirth: " + w.PlaceOfBirth + "\n");
		outputstring = outputstring.append("   PlaceOfResidence - StructuredPlace:\n");
		outputstring = outputstring.append("      City: " + w.City + "\n");
		outputstring = outputstring.append("      Country: " + w.Country + "\n");
		outputstring = outputstring.append("      Street: " + w.Street + "\n");
		outputstring = outputstring.append("      ZipCode: " + w.ZipCode + "\n");


		srand((unsigned)time(NULL));
		int u = (double)rand() / (RAND_MAX + 1) * (2000000000 - 1000000) + 1000000;
		std::string su = std::to_string(u);
		w.personalStyledString = su;
		w.AcademicTitle = su;
		w.ArtisticName = su;
		w.BirthName = su;
		w.DateOfBirth = su;
		w.DocumentType = su;
		w.FamilyNames = su;
		w.GivenNames = su;
		w.IssuingState = su;
		w.Nationality = su;
		w.PlaceOfBirth = su;
		w.City = su;
		w.Country = su;
		w.Street = su;
		w.ZipCode = su;

		//StringToWString(strText, outputstring);
		CA2W ca2w(outputstring.c_str(), CP_UTF8);
		strText = ca2w;
		outputstring = su;
		//strText = _T("KeePerso Key ready to use.");
		
		// Display a native Win32 message box
		::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
			MB_ICONINFORMATION | MB_OK);


	}


	void PersoKeyProvider::sethwnd(HWND hWndMain)
	{
		hwndvar = hWndMain;
	}

	HWND PersoKeyProvider::gethwnd()
	{
		return hwndvar;
	}


	array<System::Byte>^ PersoKeyProvider::GetKey(KeyProviderQueryContext^ ctx)
	{
		std::string keyextension = ".kperso";
		std::basic_string<TCHAR> strText;
		HWND hWndMain = hwndvar;
		
		strText = _T("before we start - please check if AusweisApp2, cardreader, Personalausweis are ready to generate key then press ok\n\nfor your information - this 4 data from your card are used to produce the key:\nFamilyNames\nGivenNames\nDateOfBirth\nPlaceOfBirth\n");
		
		// Display a native Win32 message box
		::MessageBox(hWndMain, strText.c_str(), _T("KeePerso"),
			MB_ICONINFORMATION | MB_OK);

		runworkflow();
		return provider_key;

	}

} // namespace KeePerso
