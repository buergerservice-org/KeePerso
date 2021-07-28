// workflow.cpp
// Copyright (C) 2021 buergerservice.org e.V. <KeePerso@buergerservice.org>
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)


#define BOOST_ALL_NO_LIB

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/ssl.hpp>

#include <openssl/ssl.h>
#include <openssl/sha.h>


#include "json/json.h" //jsoncpp uses MIT-Licence

#include "workflow.h"
#include <cstdlib>
#include <iostream>
//#include <stdio.h>
//#include <stdlib.h>
#include <iomanip>



namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
using response_type = http::response<http::string_body>;

typedef ssl::stream<tcp::socket> ssl_socket;



Json::Value readjsondata(std::string inputstring)
{
    Json::Value root;
    Json::CharReaderBuilder jsonbuilder;
    Json::CharReader* jsonreader = jsonbuilder.newCharReader();
    std::string errors;

    std::cout << "parsing jsondata." << std::endl;
    bool parsingSuccessful = jsonreader->parse(inputstring.c_str(),
        inputstring.c_str() + inputstring.length(), &root, &errors);
    //delete reader;
    if (!parsingSuccessful)
    {
        std::cerr << "jsoncpp parsing errors: " << errors << std::endl;
    }
    else
    {
        std::cerr << "jsoncpp parsing successful: " << errors << std::endl;
    }

    return root;
}


std::string workflow::readjson(std::string inputstring)
{
    bool parsingSuccessful = false;

    Json::Value root;
    Json::CharReaderBuilder jsonbuilder;
    Json::CharReader* jsonreader = jsonbuilder.newCharReader();
    std::string errors;
    std::string outputstring;


    std::cout << "parsing jsondata." << std::endl;
    parsingSuccessful = jsonreader->parse(inputstring.c_str(),
        inputstring.c_str() + inputstring.length(), &root, &errors);
    //delete reader;
    if (!parsingSuccessful)
    {
        std::cerr << "jsoncpp parsing errors: " << errors << std::endl;
    }

    //std::cout << "StyledString: " << root["PersonalData"].toStyledString() << std::endl;
    personalStyledString = root["PersonalData"].toStyledString();
    //std::cout << "--------------------------" << std::endl;
    //std::cout << "Exampledata read from json" << std::endl;
    Json::Value vinfo = root["PersonalData"];
    outputstring = vinfo["FamilyNames"].asString() + vinfo["GivenNames"].asString() + vinfo["DateOfBirth"].asString();

    AcademicTitle = vinfo["AcademicTitle"].asString();
    ArtisticName = vinfo["ArtisticName"].asString();
    BirthName = vinfo["BirthName"].asString();
    DateOfBirth = vinfo["DateOfBirth"].asString();
    DocumentType = vinfo["DocumentType"].asString();
    FamilyNames = vinfo["FamilyNames"].asString();
    GivenNames = vinfo["GivenNames"].asString();
    IssuingState = vinfo["IssuingState"].asString();
    Nationality = vinfo["Nationality"].asString();

    //std::cout << "the Familyname : " << vinfo["FamilyNames"].asString() << std::endl;
    //std::cout << "the GivenName : " << vinfo["GivenNames"].asString() << std::endl;
    //std::cout << "the DateOfBirth : " << vinfo["DateOfBirth"].asString() << std::endl;
    vinfo = root["PersonalData"]["PlaceOfBirth"];
    outputstring = outputstring + vinfo["FreetextPlace"].asString();
    PlaceOfBirth = vinfo["FreetextPlace"].asString();

    vinfo = root["PersonalData"]["PlaceOfResidence"]["StructuredPlace"];
    //outputstring = outputstring + vinfo["City"].asString() + vinfo["Street"].asString();
    City = vinfo["City"].asString();
    Country = vinfo["Country"].asString();
    Street = vinfo["Street"].asString();
    ZipCode = vinfo["ZipCode"].asString();

    //std::cout << "the PlaceOfResidenceCity : " << vinfo["City"].asString() << std::endl;
    //std::cout << "the PlaceOfResidenceStreet : " << vinfo["Street"].asString() << std::endl;

    //std::cout << "outputstring: " << outputstring << std::endl;

    return outputstring;
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

void findAndReplaceBehind(std::string& data, std::string toSearch, std::string replaceStr)
{
    // Get the first occurrence
    size_t pos = data.find(toSearch);
    
    // Replace this occurrence of Sub String
    data.replace(pos+toSearch.size(), 2, replaceStr);
}

std::string workflow::getcertificate()
{
    std::string outputstring = "";
    //std::locale::global(std::locale("German_germany.UTF-8"));
    //freopen("log.txt", "a+", stdout);

    try
    {

        std::string host = "127.0.0.1";
        std::string port = "24727";
        std::string target = "/eID-Kernel";

        // The io_context is required for all I/O
        net::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver{ ioc };
        websocket::stream<tcp::socket> ws{ ioc };

        // Look up the domain name
        auto const results = resolver.resolve(host, port);
        std::cout << "connect to AusweisApp2." << std::endl;
        // Make the connection on the IP address we get from a lookup
        //std::cerr << "before connect." << std::endl;
        auto ep = net::connect(ws.next_layer(), results);
        //std::cerr << "behind connect." << std::endl;
        // Update the host_ string. This will provide the value of the
        // Host HTTP header during the WebSocket handshake.
        // See https://tools.ietf.org/html/rfc7230#section-5.4
        host += ':' + std::to_string(ep.port());

        // Set a decorator to change the User-Agent of the handshake
        ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                    //std::string(BOOST_BEAST_VERSION_STRING) +
                    //" websocket-client-coro");
                    "buergerservice.org e.V. workflowLibrary");
            }));
        response_type res;
        std::cout << "starting synchronous websocket handshake." << std::endl;
        // Perform the websocket handshake
        ws.handshake(res, host, target);

        std::stringstream strbuffer;
        strbuffer << "handshake - response: \n" << res << std::endl;
        outputstring = strbuffer.str();
        std::cout << "outputstring: " << outputstring << std::endl;
        if (outputstring.find("Error") != std::string::npos)
        {
            std::cout << "found Error, please check your cardreader!" << std::endl;
            return "e3";
        }


        // This buffer will hold the incoming message
        beast::flat_buffer buffer;

        // Read a message into our buffer
        ws.read(buffer);
        // The make_printable() function helps print a ConstBufferSequence
        std::cout << beast::make_printable(buffer.data()) << std::endl;
        //std::cout << "behind first print buffer." << std::endl;
        outputstring = beast::buffers_to_string(buffer.data());
        std::cout << "outputstring: " << outputstring << std::endl;

        //buffer.clear();
        //ws.read(buffer);
        //outputstring = beast::buffers_to_string(buffer.data());
        //std::cout << outputstring << std::endl;

        std::cout << "Send command: {\"cmd\": \"RUN_AUTH\", \"tcTokenURL\" : \"https://www.autentapp.de/AusweisAuskunft/WebServiceRequesterServlet?mode=json\"}" << std::endl;
        ws.write(net::buffer(std::string("{\"cmd\": \"RUN_AUTH\", \"tcTokenURL\" : \"https://www.autentapp.de/AusweisAuskunft/WebServiceRequesterServlet?mode=json\"}")));
        buffer.clear();
        ws.read(buffer);
        std::cout << beast::make_printable(buffer.data()) << std::endl;

        outputstring = beast::buffers_to_string(buffer.data());
        std::cout << outputstring << std::endl;
        while (outputstring.find("\"AUTH\"") == std::string::npos)
        {
            buffer.clear();
            ws.read(buffer);
            outputstring = beast::buffers_to_string(buffer.data());
            std::cout << outputstring << std::endl;
            if (outputstring.find("error") != std::string::npos)
            {
                return "e1";
            }
        }
        std::cout << "found message AUTH, ok we continue." << std::endl;

        outputstring = beast::buffers_to_string(buffer.data());
        std::cout << outputstring << std::endl;
        while (outputstring.find("\"ACCESS_RIGHTS\"") == std::string::npos)
        {
            buffer.clear();
            ws.read(buffer);
            outputstring = beast::buffers_to_string(buffer.data());
            std::cout << outputstring << std::endl;
            if (outputstring.find("error") != std::string::npos)
            {
                return "e1";
            }
        }
        std::cout << "found message ACCESS_RIGHTS, ok we continue." << std::endl;


        std::cout << "Send command: {\"cmd\": \"GET_CERTIFICATE\"}" << std::endl;
        ws.write(net::buffer(std::string("{\"cmd\": \"GET_CERTIFICATE\"}")));
        buffer.clear();
        ws.read(buffer);
        outputstring = beast::buffers_to_string(buffer.data());
        std::cout << outputstring << std::endl;

        while (outputstring.find("\"CERTIFICATE\"") == std::string::npos)
        {
            buffer.clear();
            ws.read(buffer);
            outputstring = beast::buffers_to_string(buffer.data());
            std::cout << outputstring << std::endl;
            if (outputstring.find("error") != std::string::npos)
            {
                return "e1";
            }
        }
        std::cout << "found message CERTIFICATE, ok we continue" << std::endl;

        //findAndReplaceAll(outputstring, "\\u00fc", "ue");
        //findAndReplaceAll(outputstring, "\\u00e4", "ae");
        //findAndReplaceAll(outputstring, "\\u00df", "ss");

        Json::Value certificatedata;
        certificatedata = readjsondata(outputstring);
        std::cout << "StyledString: " << certificatedata["description"].toStyledString() << std::endl;
        std::cout << "issuerName: " << certificatedata["description"]["issuerName"].asString() << std::endl;
        std::cout << "issuerUrl: " << certificatedata["description"]["issuerUrl"].asString() << std::endl;
        std::cout << "purpose: " << certificatedata["description"]["purpose"].asString() << std::endl;
        std::cout << "subjectName: " << certificatedata["description"]["subjectName"].asString() << std::endl;
        std::cout << "subjectUrl: " << certificatedata["description"]["subjectUrl"].asString() << std::endl;
        std::cout << "termsOfUsage: " << certificatedata["description"]["termsOfUsage"].asString() << std::endl;
        std::cout << "effectiveDate: " << certificatedata["validity"]["effectiveDate"].asString() << std::endl;
        std::cout << "expirationDate: " << certificatedata["validity"]["expirationDate"].asString() << std::endl;

        certificateStyledString= certificatedata["description"].toStyledString();
        issuerName= certificatedata["description"]["issuerName"].asString();
        issuerUrl= certificatedata["description"]["issuerUrl"].asString();
        purpose= certificatedata["description"]["purpose"].asString();
        subjectName= certificatedata["description"]["subjectName"].asString();
        subjectUrl= certificatedata["description"]["subjectUrl"].asString();
        termsOfUsage= certificatedata["description"]["termsOfUsage"].asString();
        //findAndReplaceBehind(termsOfUsage, "Hinweis auf die f", "ue");
        //findAndReplaceBehind(termsOfUsage, "Diensteanbieter zust", "ae");
        //findAndReplaceBehind(termsOfUsage, "Die Landesbeauftragte f", "ue");
        //findAndReplaceBehind(termsOfUsage, "Arndtstra", "ss");
        
        //termsOfUsage = certificateStyledString.substr(certificateStyledString.find("termsOfUsage") + 16);
        //termsOfUsage = termsOfUsage.substr(0, termsOfUsage.find("}")-1);
        //findAndReplaceAll(termsOfUsage, "\\u00fc", "ue");
        //findAndReplaceAll(termsOfUsage, "\\u00e4", "ae");
        //findAndReplaceAll(termsOfUsage, "\\u00df", "ss");
        //findAndReplaceAll(termsOfUsage, "\\r\\n", "   ");

        effectiveDate= certificatedata["validity"]["effectiveDate"].asString();
        expirationDate= certificatedata["validity"]["expirationDate"].asString();
        // Close the WebSocket connection
        if (ws.is_open()) {
            ws.close(websocket::close_code::normal);
            std::cout << "closed websocketconnection." << std::endl;
        }

    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        //return EXIT_FAILURE;
        return "e1";
    }
    //return EXIT_SUCCESS;

    //fclose(stdout);

    return outputstring;
}


// https://stackoverflow.com/questions/2262386/generate-sha256-with-openssl-and-c/10632725
std::string sha256(const std::string str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}


std::string workflow::getkeypad()
{
    std::string outputstring = "";

    //freopen("log.txt", "a+", stdout);

    try
    {

        std::string host = "127.0.0.1";
        std::string port = "24727";
        std::string target = "/eID-Kernel";

        // The io_context is required for all I/O
        net::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver{ ioc };
        websocket::stream<tcp::socket> ws{ ioc };

        // Look up the domain name
        auto const results = resolver.resolve(host, port);
        std::cout << "connect to AusweisApp2." << std::endl;
        // Make the connection on the IP address we get from a lookup
        //std::cerr << "before connect." << std::endl;
        auto ep = net::connect(ws.next_layer(), results);
        //std::cerr << "behind connect." << std::endl;
        // Update the host_ string. This will provide the value of the
        // Host HTTP header during the WebSocket handshake.
        // See https://tools.ietf.org/html/rfc7230#section-5.4
        host += ':' + std::to_string(ep.port());

        // Set a decorator to change the User-Agent of the handshake
        ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                    //std::string(BOOST_BEAST_VERSION_STRING) +
                    //" websocket-client-coro");
                    "buergerservice.org e.V. workflowLibrary");
            }));
        response_type res;
        std::cout << "starting synchronous websocket handshake." << std::endl;
        // Perform the websocket handshake
        ws.handshake(res, host, target);

        std::stringstream strbuffer;
        strbuffer << "handshake - response: \n" << res << std::endl;
        outputstring = strbuffer.str();
        std::cout << "outputstring: " << outputstring << std::endl;
        if (outputstring.find("Error") != std::string::npos)
        {
            std::cout << "found Error, please check your cardreader!" << std::endl;
            return "e3";
        }

        std::string ver1 = outputstring.substr(outputstring.find("AusweisApp2") + 12, 1);
        std::cout << "ver1: " << ver1 << std::endl;
        std::string ver2 = outputstring.substr(outputstring.find("AusweisApp2") + 14, 2);
        std::cout << "ver2: " << ver2 << std::endl;
        if (std::stoi(ver1) < 1 || std::stoi(ver2) < 22)
        {
            std::cout << "AusweisApp2-version less 1.22.*, please update to new AusweisApp2!" << std::endl;
            return "e4";
        }

        // This buffer will hold the incoming message
        beast::flat_buffer buffer;

        // Read a message into our buffer
        ws.read(buffer);
        // The make_printable() function helps print a ConstBufferSequence
        //std::cout << beast::make_printable(buffer.data()) << std::endl;
        //std::cout << "behind first print buffer." << std::endl;
        //outputstring = beast::buffers_to_string(buffer.data());
        //std::cout << "outputstring: " << outputstring << std::endl;


        Sleep(1000);
        std::cout << "Send command: {\"cmd\": \"GET_READER_LIST\"}" << std::endl;
        ws.write(net::buffer(std::string("{\"cmd\": \"GET_READER_LIST\"}")));
        buffer.clear();
        ws.read(buffer);
        outputstring = beast::buffers_to_string(buffer.data());
        std::cout << outputstring << std::endl;

        while (outputstring.find("\"READER_LIST\"") == std::string::npos)
        {
            buffer.clear();
            ws.read(buffer);
            outputstring = beast::buffers_to_string(buffer.data());
            std::cout << outputstring << std::endl;
        }
        std::cout << "found message READER_LIST, ok we continue" << std::endl;

        Json::Value readerdata;
        readerdata = readjsondata(outputstring);
        std::cout << "StyledString: " << readerdata["reader"].toStyledString() << std::endl;
        std::cout << "readers numbers " << readerdata["reader"].size() << std::endl;

        if (readerdata["reader"].size() == 0)
        {
            std::cout << "found no cardreader, please check your cardreader!" << std::endl;
            return "e7";
        }

        bool foundcardactivated = false;

        for (int i = 0; i < readerdata["reader"].size(); i++)
        {
            std::cout << "reader nr " << i << ": " << " name: " << readerdata["reader"][i]["name"] << std::endl;
            std::cout << "attached nr " << i << ": " << readerdata["reader"][i]["attached"].asString() << std::endl;
            if (readerdata["reader"][i]["attached"])
            {
                std::cout << "   card deactivated nr " << i << ": " << readerdata["reader"][i]["card"]["deactivated"].asString() << std::endl;
                if (readerdata["reader"][i]["card"]["deactivated"])
                {
                    std::cout << "   card ative nr " << i << std::endl;
                    //this is the activated reader
                    std::cout << "   card retrycounter:" << readerdata["reader"][i]["card"]["retryCounter"].asString() << std::endl;
                    if (readerdata["reader"][i]["card"]["retryCounter"] == 3)
                    {
                        //this is the activated reader with retrycounter==3
                        std::cout << "keypad: " << readerdata["reader"][i]["keypad"].asString() << std::endl;
                        outputstring = readerdata["reader"][i]["keypad"].asString();
                        foundcardactivated = true;
                    }
                    else
                    {
                        std::cout << "found message retryCounter!=3, please start a selfauthentication direct with AusweisApp2!" << std::endl;
                        return "e5";
                    }
                }
            }
        }

        if (!foundcardactivated)
        {
            std::cout << "found no active card, please check your Personalausweis!" << std::endl;
            return "e2";
        }
        else
        {
            std::cout << "found message for active card, ok we continue." << std::endl;
        }

        /*
        if (outputstring.find("retryCounter\":3") == std::string::npos)
        {
            std::cout << "found message retryCounter!=3, please start a selfauthentication direct with AusweisApp2!" << std::endl;
            return "e5";
        }
        else
        {
            std::cout << "found message retryCounter:3, ok we continue." << std::endl;
        }

        if (outputstring.find("keypad\":false") != std::string::npos)
        {
            std::cout << "found keypad:false" << std::endl;
            outputstring = "false";
            //workflow::keypad = false;
        }
        if (outputstring.find("keypad\":true") != std::string::npos)
        {
            std::cout << "found keypad:true" << std::endl;
            outputstring = "true";
            //workflow::keypad = true;
        }
        */

        // Close the WebSocket connection
        if (ws.is_open()) {
            ws.close(websocket::close_code::normal);
            std::cout << "closed websocketconnection." << std::endl;
        }

    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        //return EXIT_FAILURE;
        return "e1";
    }
    //return EXIT_SUCCESS;

    //fclose(stdout);

    return outputstring;
}


std::string workflow::startworkflow(std::string PINstring)
{
    std::string outputstring;
    std::string errors;

    //freopen("log.txt", "a+", stdout);


    try
    {

        std::string host = "127.0.0.1";
        std::string port = "24727";
        std::string target = "/eID-Kernel";

        // The io_context is required for all I/O
        net::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver{ ioc };
        websocket::stream<tcp::socket> ws{ ioc };

        // Look up the domain name
        auto const results = resolver.resolve(host, port);
        std::cout << "connect to AusweisApp2." << std::endl;
        // Make the connection on the IP address we get from a lookup
        //std::cerr << "before connect." << std::endl;
        auto ep = net::connect(ws.next_layer(), results);
        //std::cerr << "behind connect." << std::endl;
        // Update the host_ string. This will provide the value of the
        // Host HTTP header during the WebSocket handshake.
        // See https://tools.ietf.org/html/rfc7230#section-5.4
        host += ':' + std::to_string(ep.port());

        // Set a decorator to change the User-Agent of the handshake
        ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                    //std::string(BOOST_BEAST_VERSION_STRING) +
                    //" websocket-client-coro");
                    "buergerservice.org e.V. KeePass2 Plugin KeePerso");
            }));
        response_type res;
        std::cout << "starting synchronous websocket handshake." << std::endl;
        // Perform the websocket handshake
        ws.handshake(res, host, target);
        //std::cout << "handshake - response: \n" << res << std::endl;

        std::stringstream strbuffer;
        strbuffer << "handshake - response: \n" << res << std::endl;
        outputstring = strbuffer.str();
        std::cout << "outputstring: " << outputstring << std::endl;
        if (outputstring.find("Error") != std::string::npos)
        {
            std::cout << "found Error, please check your cardreader!" << std::endl;
            return "e3";
        }

        std::string ver1 = outputstring.substr(outputstring.find("AusweisApp2") + 12, 1);
        std::cout << "ver1: " << ver1 << std::endl;
        std::string ver2 = outputstring.substr(outputstring.find("AusweisApp2") + 14, 2);
        std::cout << "ver2: " << ver2 << std::endl;
        if (std::stoi(ver1) < 1 || std::stoi(ver2) < 22)
        {
            std::cout << "AusweisApp2-version less 1.22.*, please update to new AusweisApp2!" << std::endl;
            return "e4";
        }

        // This buffer will hold the incoming message
        beast::flat_buffer buffer;

        // Read a message into our buffer
        ws.read(buffer);
        // The make_printable() function helps print a ConstBufferSequence
        //std::cout << beast::make_printable(buffer.data()) << std::endl;
        //std::cout << "behind first print buffer." << std::endl;
        //outputstring = beast::buffers_to_string(buffer.data());
        //std::cout << "outputstring: " << outputstring << std::endl;


        Sleep(1000);
        std::cout << "Send command: {\"cmd\": \"GET_READER_LIST\"}" << std::endl;
        ws.write(net::buffer(std::string("{\"cmd\": \"GET_READER_LIST\"}")));
        buffer.clear();
        ws.read(buffer);

        outputstring = beast::buffers_to_string(buffer.data());
        std::cout << outputstring << std::endl;
        while (outputstring.find("\"READER_LIST\"") == std::string::npos)
        {
            buffer.clear();
            ws.read(buffer);
            outputstring = beast::buffers_to_string(buffer.data());
            std::cout << outputstring << std::endl;
            if (outputstring.find("error") != std::string::npos)
            {
                std::cout << "found error, exiting." << std::endl;
                //outputstring = "e1";
                return "e1";
            }
        }
        std::cout << "found message READER_LIST, ok we continue" << std::endl;


        Json::Value readerdata;
        readerdata = readjsondata(outputstring);
        std::cout << "StyledString: " << readerdata["reader"].toStyledString() << std::endl;
        std::cout << "readers number " << readerdata["reader"].size() << std::endl;

        if (readerdata["reader"].size() == 0)
        {
            std::cout << "found no cardreader, please check your cardreader!" << std::endl;
            return "e7";
        }

        bool foundcardactivated = false;

        for (int i = 0; i < readerdata["reader"].size(); i++)
        {
            std::cout << "reader nr " << i << ": " << " name: " << readerdata["reader"][i]["name"] << std::endl;
            std::cout << "attached nr " << i << ": " << readerdata["reader"][i]["attached"].asString() << std::endl;
            if (readerdata["reader"][i]["attached"])
            {
                std::cout << "   card deactivated nr " << i << ": " << readerdata["reader"][i]["card"]["deactivated"].asString() << std::endl;
                if (readerdata["reader"][i]["card"]["deactivated"])
                {
                    std::cout << "   card ative nr " << i << std::endl;
                    //this is the activated reader
                    std::cout << "   card retrycounter:" << readerdata["reader"][i]["card"]["retryCounter"].asString() << std::endl;
                    if (readerdata["reader"][i]["card"]["retryCounter"] == 3)
                    {
                        //this is the activated reader with retrycounter==3
                        std::cout << "keypad: " << readerdata["reader"][i]["keypad"].asString() << std::endl;
                        outputstring = readerdata["reader"][i]["keypad"].asString();
                        foundcardactivated = true;
                    }
                    else
                    {
                        std::cout << "found message retryCounter!=3, please start a selfauthentication direct with AusweisApp2!" << std::endl;
                        //return "e5";
                        //outputstring = "e5";
                        return "e5";
                    }
                }
            }
        }

        if (!foundcardactivated)
        {
            std::cout << "found no active card, please check your Personalausweis!" << std::endl;
            //return "e2";
            //outputstring = "e2";
            return "e2";
        }
        else
        {
            std::cout << "found message for active card, ok we continue." << std::endl;
        }

        // --- now the websocketconnection is online      
        std::cout << "workflow 1 - Minimal successful authentication (in docu point 6.1):\n" << std::endl;

        std::cout << "Send command: {\"cmd\": \"RUN_AUTH\", \"tcTokenURL\" : \"https://www.autentapp.de/AusweisAuskunft/WebServiceRequesterServlet?mode=json\"}" << std::endl;
        ws.write(net::buffer(std::string("{\"cmd\": \"RUN_AUTH\", \"tcTokenURL\" : \"https://www.autentapp.de/AusweisAuskunft/WebServiceRequesterServlet?mode=json\"}")));
        buffer.clear();
        ws.read(buffer);
        std::cout << beast::make_printable(buffer.data()) << std::endl;

        outputstring = beast::buffers_to_string(buffer.data());
        std::cout << outputstring << std::endl;
        while (outputstring.find("\"AUTH\"") == std::string::npos)
        {
            buffer.clear();
            ws.read(buffer);
            outputstring = beast::buffers_to_string(buffer.data());
            std::cout << outputstring << std::endl;
            if (outputstring.find("error") != std::string::npos)
            {
                return "e1";
            }
        }
        //std::cout << "found message AUTH, ok we continue." << std::endl;


        outputstring = beast::buffers_to_string(buffer.data());
        std::cout << outputstring << std::endl;
        while (outputstring.find("\"ACCESS_RIGHTS\"") == std::string::npos)
        {
            buffer.clear();
            ws.read(buffer);
            outputstring = beast::buffers_to_string(buffer.data());
            std::cout << outputstring << std::endl;
            if (outputstring.find("error") != std::string::npos)
            {
                return "e1";
            }
        }

        std::cout << "Send command: {\"cmd\": \"ACCEPT\"}" << std::endl;
        ws.write(net::buffer(std::string("{\"cmd\": \"ACCEPT\"}")));

        outputstring = beast::buffers_to_string(buffer.data());
        std::cout << outputstring << std::endl;
        while (outputstring.find("\"ENTER_PIN\"") == std::string::npos)
        {
            buffer.clear();
            ws.read(buffer);
            outputstring = beast::buffers_to_string(buffer.data());
            std::cout << outputstring << std::endl;
            if (outputstring.find("error") != std::string::npos)
            {
                return "e1";
            }
        }
        std::cout << "found message ENTER_PIN, ok we continue." << std::endl;

        //if (outputstring.find("ENTER_PIN")!=std::string::npos)
        if (outputstring.find("\"ENTER_PIN\"") != std::string::npos)
        {
            std::cout << "found message ENTER_PIN." << std::endl;

            if (outputstring.find("keypad\":false") != std::string::npos)
            {
                std::cout << "found keypad:false" << std::endl;

                //std::cout << "Send command: {\"cmd\": \"SET_PIN\", \"value\": \"" << PINstring << "\"}\n" << std::endl;
                ws.write(net::buffer(std::string("{\"cmd\": \"SET_PIN\", \"value\": \"" + PINstring + "\"}")));

            }
            else
            {
                std::cout << "keypad:true" << std::endl;

                //std::cout << "Send command: {\"cmd\": \"SET_PIN\" }\n" << std::endl;
                ws.write(net::buffer(std::string("{\"cmd\": \"SET_PIN\"}")));
            }
        }

        outputstring = beast::buffers_to_string(buffer.data());
        std::cout << outputstring << std::endl;
        while (outputstring.find("\"AUTH\"") == std::string::npos)
        {
            buffer.clear();
            ws.read(buffer);
            outputstring = beast::buffers_to_string(buffer.data());
            std::cout << outputstring << std::endl;
            if (outputstring.find("error") != std::string::npos)
            {
                return "e1";
            }
        }
        std::string datastring = outputstring;

        //std::cout << datastring << "\n" << std::endl;
        outputstring = datastring.substr(datastring.find("https"));
        datastring = outputstring.substr(0, outputstring.find("\""));
        //std::cout << datastring << std::endl;

        //std::string httpstring = "{\"msg\":\"AUTH\",\"result\":{\"major\":\"http://www.bsi.bund.de/ecard/api/1.1/resultmajor#ok\"},\"url\":\"https://www.autentapp.de/AusweisAuskunft/WebServiceReceiverServlet?refID=123456&mode=json\"}";
        std::string outs;

        outputstring = datastring.substr(datastring.find("https"));
        outs = outputstring.substr(0, outputstring.find("\""));
        //std::cout << outs << std::endl;

        std::string url = outs.substr(8);
        std::cout << "url = " << url << std::endl;
        host = url.substr(0, url.find("/"));
        std::cout << "host = " << host << std::endl;
        target = url.substr(url.find("/"));
        std::cout << "target = " << target << std::endl;

        port = "443"; //=https
        //read https
        ssl::context ctx{ ssl::context::sslv23_client };
        ctx.set_verify_mode(ssl::verify_none);
        ssl::stream<tcp::socket> str{ ioc, ctx };
        auto endpts = resolver.resolve(host, port);
        std::cout << "connect to resulthost." << std::endl;
        net::connect(str.next_layer(), endpts.begin(), endpts.end());
        std::cout << "starting handshake." << std::endl;
        str.handshake(ssl::stream_base::client);
        http::request<http::string_body> srequest{ http::verb::get, target, 11 };
        srequest.set(http::field::host, host);
        http::write(str, srequest);
        http::response<http::string_body> sresp;
        boost::beast::flat_buffer bbuffer;
        http::read(str, bbuffer, sresp);
        //std::cout << "response = " << sresp.body() << std::endl;

        //parse json
        outputstring=readjson(sresp.body().c_str());
        datastring = outputstring; 

        //generate hash
        outputstring = sha256(datastring);
        //std::cout << "sha256 hash = " << outputstring << std::endl;

        // Close the WebSocket connection
        if (ws.is_open()) {
            ws.close(websocket::close_code::normal);
            std::cout << "closed websocketconnection." << std::endl;
        }
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        //return EXIT_FAILURE;
        return "e1";
    }
    //return EXIT_SUCCESS;

    //fclose(stdout);

    return outputstring;
}