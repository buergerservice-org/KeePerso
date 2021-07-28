// workflow.h
// Copyright (C) 2021 buergerservice.org e.V. <KeePerso@buergerservice.org>
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string>


class workflow
{
	public:
		std::string readjson(std::string);
		std::string getkeypad();
		std::string getcertificate();
		std::string startworkflow(std::string PINstring);
		
		//personaldata
		std::string personalStyledString;
		std::string AcademicTitle;
		std::string ArtisticName;
		std::string BirthName;
		std::string DateOfBirth;
		std::string DocumentType;
		std::string FamilyNames;
		std::string GivenNames;
		std::string IssuingState;
		std::string Nationality;
		//personaldata PlaceOfBirth
		std::string PlaceOfBirth;
		//personaldata PlaceOfResidence
		std::string City;
		std::string Country;
		std::string Street;
		std::string ZipCode;

		// certificate
		std::string certificateStyledString;
		// certificate description
		std::string issuerName;
		std::string issuerUrl;
		std::string purpose;
		std::string subjectName;
		std::string subjectUrl;
		std::string termsOfUsage;
		// certificate validity
		std::string effectiveDate;
		std::string expirationDate;

	private:
		bool keypad;

};

