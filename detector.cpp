//
// Copyright 2011-2015 Ettus Research LLC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

//UHD Libraries
#include <uhd/utils/thread_priority.hpp>
#include <uhd/convert.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
//BoostLibraries
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/thread/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
//C++ Libraries
#include <iostream>
#include <complex>
#include <cstdlib>
#include <numeric>
#include <fstream>
#include <unistd.h>
#include <functional>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <cstdio>
#include <vector>
#include <time.h>
#include <tuple>
#include <boost/tuple/tuple.hpp>
//FFTW Libraries
#include <fftw3.h>
//SensorUp Libraries
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <chrono>

pthread_mutex_t lock;
namespace po = boost::program_options;
const double CLOCK_TIMEOUT = 1000;  // 1000mS timeout for external clock locking
const double INIT_DELAY    = 0.0001;  // 50mS initial delay before transmit
std::ofstream fppow0;
std::ofstream fppow1;
std::ofstream fpspec0;
std::ofstream fpspec1;
std::ofstream fppsd0;
std::ofstream fppsd1;
//std::ofstream fpmetadata;
std::ofstream fptimeoutData;
int old_minute = 500;
using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;
using boost::asio::ip::tcp;

boost::tuple<std::string, std::string> createDatastream(std::string write_to, std::string func, int index)
        {
    std::string jsonStr, host_name, portno, header, id, result, p_time;
    int position_s, position_e;
    //JSON body
    ptree root, info;
    boost::asio::streambuf request;
    std::ostringstream strline;
    std::ostream request_stream(&request);

    std::ostringstream buf;

    time_t rawtime = time(0);
    tm *ltm = localtime(&rawtime);
    int p_hour = ltm->tm_hour;
    int p_minute = ltm->tm_min;
    int p_second = ltm->tm_sec;
    int p_year = ltm->tm_year + 1900;
    int p_day = ltm->tm_mday;
    int p_month = ltm->tm_mon + 1;
    //2017-03-22T16:05:07.000Z
    boost::format itime("%d-%02d-%02dT%02d:%02d:%02dZ");
    itime %p_year %p_month %p_day %p_hour %p_minute %p_second;
    p_time = boost::str(itime);

    if(write_to == "SensorUp")
    {
        host_name = "scratchpad.sensorup.com";
        portno = "80";

        if(index == 0){
            if(func == "Power Profile")
                jsonStr = "\n{  \n   \"name\": \"L1/E1 Interference Power Profile\",\n   \"description\": \"Power Profile measurement\",\n          \"observationType\": \"http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement\",\n            \"unitOfMeasurement\": {\n      \"name\": \"Signal-to-Noise-Ratio\",\n      \"symbol\": \"dB\",\n           \"definition\": \"http://www.qudt.org/qudt/owl/1.0.0/unit/Instances.html\"\n   },\n   \"Sensor\": {\n           \"@iot.id\": 779055  \n   },\n   \"ObservedProperty\": {\n      \"@iot.id\": 779057  \n   },\n   \"Thing\":         {\n      \"@iot.id\": 779052  \n   }\n}";
            else if (func == "Power Spectrum")
                jsonStr = "\n{  \n   \"name\": \"L1/E1 Interference Power Spectrum\",\n   \"description\": \"Power Spectrum measurement\",\n        \"observationType\": \"http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement\",\n            \"unitOfMeasurement\": {\n      \"name\": \"Frequency-Magnitude-Ratio\",\n      \"symbol\": \"dB\",\n           \"definition\": \"http://www.qudt.org/qudt/owl/1.0.0/unit/Instances.html\"\n   },\n   \"Sensor\": {\n           \"@iot.id\": 779055  \n   },\n   \"ObservedProperty\": {\n      \"@iot.id\": 779057  \n   },\n   \"Thing\":         {\n      \"@iot.id\": 779052  \n   }\n}";
            else if (func == "Spectrogram")
                jsonStr = "\n{  \n   \"name\": \"L1/E1 Interference Spectrogram\",\n   \"description\": \"Spectrogram measurement\",\n          \"observationType\": \"http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement\",\n            \"unitOfMeasurement\": {\n      \"name\": \"Frequency-Magnitude-Ratio\",\n      \"symbol\": \"dB\",\n           \"definition\": \"http://www.qudt.org/qudt/owl/1.0.0/unit/Instances.html\"\n   },\n   \"Sensor\": {\n           \"@iot.id\": 779055  \n   },\n   \"ObservedProperty\": {\n      \"@iot.id\": 779057  \n   },\n   \"Thing\":         {\n      \"@iot.id\": 779052  \n   }\n}";
        }
        else
        {
            if(func == "Power Profile")
                jsonStr = "\n{  \n   \"name\": \"L5/E5 Interference Power Profile\",\n   \"description\": \"Power Profile measurement\",\n          \"observationType\": \"http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement\",\n            \"unitOfMeasurement\": {\n      \"name\": \"Signal-to-Noise-Ratio\",\n      \"symbol\": \"dB\",\n           \"definition\": \"http://www.qudt.org/qudt/owl/1.0.0/unit/Instances.html\"\n   },\n   \"Sensor\": {\n           \"@iot.id\": 779055  \n   },\n   \"ObservedProperty\": {\n      \"@iot.id\": 779057  \n   },\n   \"Thing\":         {\n      \"@iot.id\": 779052  \n   }\n}";
            else if (func == "Power Spectrum")
                jsonStr = "\n{  \n   \"name\": \"L5/E5 Interference Power Spectrum\",\n   \"description\": \"Power Spectrum measurement\",\n        \"observationType\": \"http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement\",\n            \"unitOfMeasurement\": {\n      \"name\": \"Frequency-Magnitude-Ratio\",\n      \"symbol\": \"dB\",\n           \"definition\": \"http://www.qudt.org/qudt/owl/1.0.0/unit/Instances.html\"\n   },\n   \"Sensor\": {\n           \"@iot.id\": 779055  \n   },\n   \"ObservedProperty\": {\n      \"@iot.id\": 779057  \n   },\n   \"Thing\":         {\n      \"@iot.id\": 779052  \n   }\n}";
            else if (func == "Spectrogram")
                jsonStr = "\n{  \n   \"name\": \"L5/E5 Interference Spectrogram\",\n   \"description\": \"Spectrogram measurement\",\n          \"observationType\": \"http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement\",\n            \"unitOfMeasurement\": {\n      \"name\": \"Frequency-Magnitude-Ratio\",\n      \"symbol\": \"dB\",\n           \"definition\": \"http://www.qudt.org/qudt/owl/1.0.0/unit/Instances.html\"\n   },\n   \"Sensor\": {\n           \"@iot.id\": 779055  \n   },\n   \"ObservedProperty\": {\n      \"@iot.id\": 779057  \n   },\n   \"Thing\":         {\n      \"@iot.id\": 779052  \n   }\n}";
        }
        info.put("placeholder", "value");
        info.put("value", "hi!");
        info.put("module","value");
        root.put_child("exception", info);

        //headers and connection request
        request_stream << "POST /OGCSensorThings/v1.0/Datastreams HTTP/1.1\r\n";
        request_stream << "Host: " << "scratchpad.sensorup.com" << "\r\n";
        request_stream << "Content-Type: application/json \r\n";
        request_stream << "Accept: application/json \r\n";
        request_stream << "St-P-Access-Token: 353e95b3-1841-4c6e-b54a-d4886534563e \r\n";
        request_stream << "Cache-Control: no-cache";
        request_stream << "Accept: application/json \r\n";
        request_stream << "Content-Length: " << jsonStr.length() << "\r\n";
        request_stream << "Connection: close \r\n\r\n";
        request_stream << jsonStr;

    }
    else if(write_to == "Fraunhofer")
    {
        host_name = "212.68.73.76";
        portno = "8080";

        if(index == 0)
        {
            if(func == "Power Profile")
                jsonStr = "\n{  \n   \"name\": \"L1/E1 Interference Power Profile\",\n   \"description\": \"Power Profile measurement\",\n          \"observationType\": \"http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement\",\n            \"unitOfMeasurement\": {\n      \"name\": \"Signal-to-Noise-Ratio\",\n      \"symbol\": \"dB\",\n           \"definition\": \"http://www.qudt.org/qudt/owl/1.0.0/unit/Instances.html\"\n   },\n   \"Sensor\": {\n           \"@iot.id\": 3  \n   },\n   \"ObservedProperty\": {\n      \"@iot.id\": 2  \n   },\n   \"Thing\":       {\n      \"@iot.id\": 2  \n   }\n}";
            else if(func == "Power Spectrum")
                jsonStr = "\n{  \n   \"name\": \"L1/E1 Interference Power Spectrum\",\n   \"description\": \"Power Spectrum measurement\",\n        \"observationType\": \"http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement\",\n            \"unitOfMeasurement\": {\n      \"name\": \"Frequency-Magnitude-Ratio\",\n      \"symbol\": \"dB\",\n           \"definition\": \"http://www.qudt.org/qudt/owl/1.0.0/unit/Instances.html\"\n   },\n   \"Sensor\": {\n           \"@iot.id\": 3  \n   },\n   \"ObservedProperty\": {\n      \"@iot.id\": 2  \n   },\n   \"Thing\":       {\n      \"@iot.id\": 2  \n   }\n}";
            else if(func == "Spectrogram")
                jsonStr = "\n{  \n   \"name\": \"L1/E1 Interference Spectrogram\",\n   \"description\": \"Spectrogram measurement\",\n          \"observationType\": \"http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement\",\n            \"unitOfMeasurement\": {\n      \"name\": \"Frequency-Magnitude-Ratio\",\n      \"symbol\": \"dB\",\n           \"definition\": \"http://www.qudt.org/qudt/owl/1.0.0/unit/Instances.html\"\n   },\n   \"Sensor\": {\n           \"@iot.id\": 3  \n   },\n   \"ObservedProperty\": {\n      \"@iot.id\": 2  \n   },\n   \"Thing\":       {\n      \"@iot.id\": 2  \n   }\n}";
        }
        else
        {
            if(func == "Power Profile")
                jsonStr = "\n{  \n   \"name\": \"L5/E5 Interference Power Profile\",\n   \"description\": \"Power Profile measurement\",\n          \"observationType\": \"http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement\",\n            \"unitOfMeasurement\": {\n      \"name\": \"Signal-to-Noise-Ratio\",\n      \"symbol\": \"dB\",\n           \"definition\": \"http://www.qudt.org/qudt/owl/1.0.0/unit/Instances.html\"\n   },\n   \"Sensor\": {\n           \"@iot.id\": 3  \n   },\n   \"ObservedProperty\": {\n      \"@iot.id\": 2  \n   },\n   \"Thing\":       {\n      \"@iot.id\": 2  \n   }\n}";
            else if(func == "Power Spectrum")
                jsonStr = "\n{  \n   \"name\": \"L5/E5 Interference Power Spectrum\",\n   \"description\": \"Power Spectrum measurement\",\n        \"observationType\": \"http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement\",\n            \"unitOfMeasurement\": {\n      \"name\": \"Frequency-Magnitude-Ratio\",\n      \"symbol\": \"dB\",\n           \"definition\": \"http://www.qudt.org/qudt/owl/1.0.0/unit/Instances.html\"\n   },\n   \"Sensor\": {\n           \"@iot.id\": 3  \n   },\n   \"ObservedProperty\": {\n      \"@iot.id\": 2  \n   },\n   \"Thing\":       {\n      \"@iot.id\": 2  \n   }\n}";
            else if(func == "Spectrogram")
                jsonStr = "\n{  \n   \"name\": \"L5/E5 Interference Spectrogram\",\n   \"description\": \"Spectrogram measurement\",\n          \"observationType\": \"http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement\",\n            \"unitOfMeasurement\": {\n      \"name\": \"Frequency-Magnitude-Ratio\",\n      \"symbol\": \"dB\",\n           \"definition\": \"http://www.qudt.org/qudt/owl/1.0.0/unit/Instances.html\"\n   },\n   \"Sensor\": {\n           \"@iot.id\": 3  \n   },\n   \"ObservedProperty\": {\n      \"@iot.id\": 2  \n   },\n   \"Thing\":       {\n      \"@iot.id\": 2  \n   }\n}";
        }

        //headers and connection request
        request_stream << "POST /SensorThingsServer-1.0/v1.0/Datastreams HTTP/1.1\r\n";
        request_stream << "Host: " << "212.68.73.76:8080" << "\r\n";
        request_stream << "Content-Type: application/json \r\n";
        //request_stream << "User-Agent: C/1.0\r\n";
        //request_stream << "St-P-Access-Token: 353e95b3-1841-4c6e-b54a-d4886534563e \r\n";
        request_stream << "Cache-Control: no-cache";
        request_stream << "Accept: application/json \r\n";
        request_stream << "Content-Length: " << jsonStr.length() << "\r\n";
        request_stream << "Connection: close \r\n\r\n";
        request_stream << jsonStr;
    }

    boost::asio::io_service sv;

    //retrieve end points corresponding to server name
    tcp::resolver resolver(sv);
    tcp::resolver::query query(host_name, portno);//, boost::asio::ip::resolver_query_base::numeric_service);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    tcp::resolver::iterator end;

    //try the endpoints until a successful connection is established
    //handle associated errors encountered
    tcp::socket socket(sv);

    //boost::system::error_code error = boost::asio::error::host_not_found;
    try
    {
        socket.close();
        boost::asio::connect(socket, endpoint_iterator);
        //socket.connect(*endpoint_iterator);
    }
    catch (boost::system::system_error const& e)
    {
        std::cout << "Warning: could not connect : " << e.what() << std::endl;
    }

    //send request
    boost::asio::write(socket, request);


    //handle response from server
    boost::asio::streambuf response;
    boost::asio::read_until(socket, response, "\r\n");

    // Check that response is OK.
    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    std::getline(response_stream, status_message);
    if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
        std::cout << "Invalid response\n";;
    }
    if (status_code != 200) {
        //        std::cout << "Response returned with status code " << status_code << "\n";
    }


    // Read the response headers, which are terminated by a blank line.
    boost::asio::read_until(socket, response, "\r\n\r\n");

    //Retrieve the Datastream id from the Loaction header
    strline << &response;
    result = strline.str();

    position_s = result.find_last_of("(") + 1;
    position_e =  result.find_last_of(")") ;

    for( int i = position_s; i < position_e; i++)
        id.push_back(result.at(i));
    //std::cout << id;

    return boost::make_tuple(id, p_time);
        }


int SendToServer(std::vector<float>&observations, std::string write, std::string Datastream_id, std::string P_Time)
{
    std::stringstream json_stream;
    std::string host_name, portno, json_str;

    //form the request
    boost::asio::streambuf request;
    std::ostream request_stream(&request);

    std::ostringstream buf;
    boost::asio::io_service sv;

    if(write =="Fraunhofer")
    {
        host_name= "212.68.73.76";
        portno = "8080";
        //std::cout<<"Datastream id:" << Datastream_id <<std::endl;

        //json_stream<<"[\n{ \n \"Datastream\": { \"@iot.id\":" << Datastream_id <<" \n }, \n \"components\": [ \n \"phenomenonTime\" ,\n \"resultTime\" , \n \"result\"\n ], \n \"dataArray@iot.count\":4096, \n \"dataArray\": [";
        json_stream<<"[\n{ \n \"Datastream\": { \"@iot.id\":" << Datastream_id <<" \n }, \n \"components\": [ \n \"phenomenonTime\" ,\n \"result\"\n ], \n \"dataArray@iot.count\":"<<observations.size()<<", \n \"dataArray\": [";
        for(unsigned int i=0; i<observations.size();i++)
        {
            if(i!=observations.size()-1)
                //json_stream<<"[\""<<P_Time<<"\"\n"<<","<<"\""<<R_Time<<"\",\n"<< observations[i] <<"],"; //json_stream<<"[" << i <<"],";
                json_stream<<"[\""<<P_Time<<"\",\n"<< observations[i] <<"],"; //json_stream<<"[" << i <<"],";
            else
                //json_stream<<"[\""<<P_Time<<"\"\n"<<","<<"\""<<R_Time<<"\",\n"<< observations[i] << "]]}]"; //json_stream<<"[" << i << "]]}]";
                json_stream<<"[\""<<P_Time<<"\",\n"<< observations[i] << "]]}]"; //json_stream<<"[" << i << "]]}]";
        }
        json_str = json_stream.str();

        //headers and connection request
        //request_stream << "POST /SensorThingsServer-1.0/v1.0/Datastreams(31)/Observations HTTP/1.1\r\n";
        request_stream << "POST /SensorThingsServer-1.0/v1.0/CreateObservations HTTP/1.1\r\n";
        request_stream << "Host: " << "212.68.73.76:8080" << "\r\n";
        request_stream << "Content-Type: application/json \r\n";
        //request_stream << "User-Agent: C/1.0\r\n";
        //request_stream << "St-P-Access-Token: 353e95b3-1841-4c6e-b54a-d4886534563e \r\n";
        request_stream << "Cache-Control: no-cache";
        request_stream << "Accept: application/json \r\n";
        request_stream << "Content-Length: " << json_str.length() << "\r\n";
        request_stream << "Connection: close \r\n\r\n";
        request_stream << json_str;
    }

    else{

        host_name= "scratchpad.sensorup.com";
        portno = "80";
        //std::cout<<"Datastream id:" << Datastream_id <<std::endl;

        json_stream<<"[\n{ \n \"Datastream\": { \"@iot.id\":" << Datastream_id <<" \n }, \n \"components\": [ \n \"result\" \n ], \n \"dataArray@iot.count\":"<<observations.size()<<", \n \"dataArray\": [";
        for(unsigned int i=0; i<observations.size();i++)
        {
            if(i!=observations.size()-1)
                json_stream<<"[" << observations[i] <<"],";
            else
                json_stream<<"[" << observations[i] << "]]}]";
        }
        json_str = json_stream.str();

        //headers and connection request
        request_stream << "POST /OGCSensorThings/v1.0/CreateObservations HTTP/1.1\r\n";
        request_stream << "Host: " << "scratchpad.sensorup.com" << "\r\n";
        request_stream << "Content-Type: application/json \r\n";
        //request_stream << "User-Agent: C/1.0\r\n";
        request_stream << "St-P-Access-Token: 353e95b3-1841-4c6e-b54a-d4886534563e \r\n";
        request_stream << "Cache-Control: no-cache";
        request_stream << "Accept: application/json \r\n";
        request_stream << "Content-Length: " << json_str.length() << "\r\n";
        request_stream << "Connection: close \r\n\r\n";
        request_stream << json_str;


    }

    //retrieve end points corresponding to server name
    tcp::resolver resolver(sv);
    tcp::resolver::query query(host_name, portno);//, boost::asio::ip::resolver_query_base::numeric_service);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    tcp::resolver::iterator end;

    //try the endpoints until a successful connection is established
    //handle associated errors encountered
    tcp::socket socket(sv);

    //boost::system::error_code error = boost::asio::error::host_not_found;
    try
    {
        socket.close();
        boost::asio::connect(socket, endpoint_iterator);
        //socket.connect(*endpoint_iterator);
    }
    catch (boost::system::system_error const& e)
    {
        std::cout << "Warning: could not connect : " << e.what() << std::endl;
    }


    //send request
    boost::asio::write(socket, request);
    return 1;
}


/***********************************************************************
 * Windowing - Hann Window
 **********************************************************************/
float hanning(int idx, int window_size) {
    return (0.5
            * (1.0 - cos(2.0 * M_PI * (double) idx / (double) (window_size - 1))));
}

/***********************************************************************
 * Spectrogram Computation
 **********************************************************************/
void calcSpectrogram(std::vector<std::complex<float> >&buffer, int STFTsize, int spectrogram_loops, int index,
        std::vector<float>& spectrogram)
{


    std::vector<std::complex<float> > buff_STFT(STFTsize);
    std::vector<std::complex<float> > buff_win(STFTsize);
    //std::vector<float> spectrogram(spectrogram_loops * STFTsize);

    fftwf_complex *in_STFT;
    fftwf_complex *out_STFT;
    fftwf_plan plan_STFT;

    for (int iFftLoop = 0; iFftLoop < spectrogram_loops; iFftLoop++)
    {
        for(int cnt=0;cnt<STFTsize;cnt++)
        {
            int iStartIndex = iFftLoop * STFTsize / 2+cnt;
            buff_win[cnt]=buffer[iStartIndex] * hanning(cnt,STFTsize);
        }

        in_STFT=(fftwf_complex*)&buff_win[0];
        out_STFT=(fftwf_complex*)&buff_STFT[0];

        //create complex one dimensional PLAN
        plan_STFT=fftwf_plan_dft_1d(STFTsize,in_STFT,out_STFT, FFTW_FORWARD, FFTW_ESTIMATE);
        //Execute FFT plan
        fftwf_execute(plan_STFT);

        //Compute power of the signal from the derived FFT
        for(int k=0;k<STFTsize;k++)
        {
            int iStartIndexOut = iFftLoop*STFTsize + k;
            spectrogram[iStartIndexOut]=real(buff_STFT[k]) * real(buff_STFT[k]) + imag(buff_STFT[k]) * imag(buff_STFT[k]);

        }

    }//end of for loop - fft computation

    fftwf_destroy_plan(plan_STFT);
}

void writeSpectrogramToFile(int index, int STFTsize, int spectrogram_loops, std::vector<float>& spectrogram, std::string itime)
{
    std::string filename;

    if(index == 0)
    {
        filename = "/home/navcert/workspace/ProcessChannel/E1-L1/Spectrogram/"+itime;
        //fpspec0.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
        fpspec0.open(filename.c_str());
        for(int iFftLoop = 0; iFftLoop < spectrogram_loops; iFftLoop++)
        {
            for(int k=0;k<STFTsize;k++)
            {
                int iStartIndexOut = iFftLoop*STFTsize + k;
                fpspec0 << spectrogram[iStartIndexOut] <<"\t";
            }
            fpspec0 << std::endl;
        }
        fpspec0.flush();
        fpspec0.close();
    }//index ==0

    else
    {
        filename = "/home/navcert/workspace/ProcessChannel/E2-L2/Spectrogram/"+itime;
        //fpspec1.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
        fpspec1.open(filename.c_str());

        for(int iFftLoop = 0; iFftLoop < spectrogram_loops; iFftLoop++)
        {
            for(int k=0;k<STFTsize;k++)
            {
                int iStartIndexOut = iFftLoop*STFTsize + k;
                fpspec1 << spectrogram[iStartIndexOut] <<"\t";
            }
            fpspec1 << std::endl;
        }
        fpspec1.flush();
        fpspec1.close();
    }//index == 1
}

/****************************************
 * Power Profile
 *
 ****************************************/

void writePowProfileToFile(int index, std::vector<float> &tempPowerProfile, int powIterator, std::string itime)
{

    std::string filename;


    if(index==0)
    {
        filename = "/home/navcert/workspace/ProcessChannel/E1-L1/powerProfile/"+itime;
        fppow0.open(filename.c_str(), std::ios_base::out | std::ios_base::app);

        for (int ii = 0;ii<powIterator;ii++)
        {
            fppow0 << tempPowerProfile[ii]<< std::endl;
            //fppow1 << pow_time_mean << std::endl;

        }
        fppow0.flush();
        fppow0.close();

    }//index == 0
    else{
        filename = "/home/navcert/workspace/ProcessChannel/E2-L2/powerProfile/"+itime;
        fppow1.open(filename.c_str(), std::ios_base::out | std::ios_base::app);
        for (int ii = 0;ii<powIterator;ii++)
        {
            fppow1 << tempPowerProfile[ii]<< std::endl;
            //fppow1 << pow_time_mean << std::endl;

        }
        fppow1.flush();
        fppow1.close();

    }//index ==1
    /*
    else{
        boost::tie(DS_id, DS_time) = createDatastream(write_medium, "Power Profile", index);
        SendToServer(tempPowerProfile, write_medium, DS_id, DS_time);
    }
     */
}



void calcPowerProfile(std::vector<std::complex<float> >&buffer, std::vector<float>&pow_time_avg, int num_rx_samps, int bufferloop_count)
{
    float buff_pow_time_real_sum = 0.0;
    float buff_pow_time_imag_sum = 0.0;
    std::vector<float> buff_pow_time_real(num_rx_samps);
    std::vector<float> buff_pow_time_imag(num_rx_samps);


    for(int j = 0; j < num_rx_samps; j++) {
        buff_pow_time_real[j] = real(buffer[j]) * real(buffer[j]);
        buff_pow_time_imag[j] = imag(buffer[j]) * imag(buffer[j]);
    }
    buff_pow_time_real_sum = std::accumulate(buff_pow_time_real.begin(), buff_pow_time_real.end(), 0.0);
    buff_pow_time_imag_sum = std::accumulate(buff_pow_time_imag.begin(), buff_pow_time_imag.end(), 0.0);
    pow_time_avg[bufferloop_count] = (buff_pow_time_real_sum + buff_pow_time_imag_sum) / num_rx_samps;

}

void doPowerDetection(float &pow_time_mean, std::vector<float>&pow_time_avg, int bufferloop_to_average)
{

    float pow_time_avg_sum = 0.0;

    pow_time_avg_sum = std::accumulate(pow_time_avg.begin(), pow_time_avg.end(), 0.0);
    pow_time_mean = pow_time_avg_sum / bufferloop_to_average;


}

/**************************************************
 *PSD Detection
 *
 * ***/
void calcPSD(std::vector<std::complex<float> >&buffer, std::vector<float>&psd_average, int num_rx_samps, int iFTsize,
        int bufferloop_count)
{
    int psd_loops = num_rx_samps/iFTsize;
    fftwf_complex *in_FT;
    fftwf_complex *out_FT;
    std::vector<std::complex<float> > buff_FT(num_rx_samps);
    std::vector<float> psd(num_rx_samps);

    // FFT averaged over 1 packet
    fftwf_plan plan_FT;

    for (int jFftLoop = 0; jFftLoop < psd_loops; jFftLoop++) {
        int jStartIndex = jFftLoop * iFTsize;
        in_FT = (fftwf_complex*) &buffer[jStartIndex];
        out_FT = (fftwf_complex*) &buff_FT[jStartIndex];

        plan_FT = fftwf_plan_dft_1d(iFTsize, in_FT, out_FT, FFTW_FORWARD, FFTW_ESTIMATE);
        fftwf_execute(plan_FT);

        for (int t = 0; t < iFTsize; t++) {
            int ndx = jStartIndex + t;
            psd[ndx] = real(buff_FT[ndx]) * real(buff_FT[ndx]) + imag(buff_FT[ndx]) * imag(buff_FT[ndx]);
        }
    }

    fftwf_destroy_plan(plan_FT);

    for (int d = 0; d < iFTsize; d++) {

        int iStartIndexOut = bufferloop_count * iFTsize + d;

        for (int counter = 0; counter < psd_loops; counter++) {
            int iter_FT = counter * iFTsize + d;
            psd_average[iStartIndexOut] += psd[iter_FT];
        }

        psd_average[iStartIndexOut] = psd_average[iStartIndexOut] / psd_loops;

    }

}

void doPsdDetection(int bufferloop_to_average, int iFTsize, std::vector<float>&psd_average, int &nPacketsAbove,
        std::vector<float>&psd_average2, int index, float &pow_time_mean)
{
    //std::vector<float> psd_average2(iFTsize);

    for(int e=0;e<iFTsize;e++)
    {
        psd_average2[e] = 0.0;

        for(int bufferloop_count=0; bufferloop_count<bufferloop_to_average; bufferloop_count++)
        {
            int jStartIndexOut = bufferloop_count * iFTsize + e;
            psd_average2[e] += psd_average[jStartIndexOut];

            psd_average[jStartIndexOut] = 0;
        }

        psd_average2[e] = psd_average2[e]/bufferloop_to_average;

    }//end of for

    pow_time_mean = 0;
    for(int i = 2000; i <=3200; i++)
        pow_time_mean += psd_average2[i];

    // calc power
    //pow_time_mean = std::accumulate(psd_average2[2000], psd_average2[3200], 0.0);
    //pow_time_mean = std::accumulate(psd_average2.begin()+2000, psd_average2.begin()+3200, 0.0);

    //pow_time_mean = std::accumulate(psd_average2.begin(), psd_average2.end(), 0.0);



}
void writePSDdetectionToFile(int index, int FTsize, std::vector<float>&psd_average2, std::string itime)
{
    std::string filename;

    if(index == 0){
        filename = "/home/navcert/workspace/ProcessChannel/E1-L1/PSD/"+itime;
        fppsd0.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
        for(int e=0; e<FTsize;e++)
            fppsd0 << psd_average2[e] <<"\t";
        fppsd0.flush();
        fppsd0.close();
    }
    else
    {
        filename = "/home/navcert/workspace/ProcessChannel/E2-L2/PSD/"+itime;
        fppsd1.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
        for(int e=0; e<FTsize;e++)
            fppsd1 << psd_average2[e] <<"\t";
        fppsd1.flush();
        fppsd1.close();
    }//index == 1


    /*else{
        boost::tie(DS_id, DS_time) = createDatastream(write_mode, "Power Spectrum", index);
        SendToServer(psd_average2, write_mode, DS_id, DS_time);
    }*/

}



std::string computeTime()
{
    std::string Time;
    time_t rawtime = time(0);
    tm *ltm = localtime(&rawtime);
    int hour = ltm->tm_hour;
    int minute = ltm->tm_min;
    int second = ltm->tm_sec;
    int year = ltm->tm_year+1900;
    int day = ltm->tm_mday;
    int month = ltm->tm_mon+1;
    boost::format itime("%d-%02d-%02dT%02d:%02d:%02dZ");
    itime %year %month %day %hour %minute %second;

    Time = boost::str(itime);
    return Time;
}
/***********************************************************************
 * Receive Task
 **********************************************************************/
void recvTask(
        uhd::usrp::multi_usrp::sptr usrp,
        uhd::rx_streamer::sptr rx_stream,
        float total_num_samps,
        int bufferloop_to_average,
        float rate,
        std::string write_to
) {
    uhd::set_thread_priority_safe();

    //Set buffers
    std::vector<std::vector<std::complex<float> > > buffs(
            usrp->get_rx_num_channels(),
            std::vector<std::complex<float> >(total_num_samps));

    //create a vector of pointers to point to each of the channel buffers
    std::vector<std::complex<float> *> buff_ptrs;
    for (size_t i = 0; i < buffs.size(); i++)
        buff_ptrs.push_back(&buffs[i].front());

    uhd::stream_cmd_t cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);
    cmd.time_spec = usrp->get_time_now() + uhd::time_spec_t(INIT_DELAY);
    //cmd.time_spec = uhd::time_spec_t();
    cmd.stream_now = true;
    rx_stream->issue_stream_cmd(cmd);

    bool overflow_message = true;

    uhd::time_spec_t last_time;
    //unsigned long long process_counter = 0;
    //unsigned long long overall_counter = 0;
    unsigned long long dropped_samps = 0;
    unsigned long long num_samps = 0;
    //  double time_requested = 20.0;
    //  boost::system_time start = boost::get_system_time();
    // unsigned long long ticks_requested = (long)(time_requested * (double)boost::posix_time::time_duration::ticks_per_second());
    //   boost::posix_time::time_duration ticks_diff;
    //   boost::system_time last_update = start;
    //   unsigned long long last_update_samps = 0;
    //   typedef std::map<size_t,size_t> SizeMap;
    //  SizeMap mapSizes;
    //  bool enable_size_map = false;


    int FTsize=4096;
    int STFTsize = 128;
    int spectrogram_loops=100;
    float P_MAX0;
    float P_MAX1;
    int nLoopsAbove0 = 0;
    int nLoopsAbove1 = 0;

    int nPacketsAbove0 = 0;
    int nPacketsAbove1 = 0;
    //int nPacketsBelow0 = 0;
    //int nPacketsBelow1 = 0;
    //int PSDdetection0 = 0;
    //int PSDdetection1 = 0;

    bool bLast0 = false;
    bool bLast1 = false;;

    std::vector<float> pow_time_avg0(bufferloop_to_average);
    std::vector<float> pow_time_avg1(bufferloop_to_average);
    std::vector<float> psd_average0(FTsize * bufferloop_to_average);
    std::vector<float> psd_average1(FTsize * bufferloop_to_average);

    std::vector<float> psd0(FTsize);
    std::vector<float> psd1(FTsize);
    std::vector<float> spectrogram0(spectrogram_loops * STFTsize);
    std::vector<float> spectrogram1(spectrogram_loops * STFTsize);
    float pow_time_mean0 = 0.0;
    float pow_time_mean1 = 0.0;

    int powIterator = 5;
    std::vector<float> powProfile0(powIterator);
    std::vector<float> powProfile1(powIterator);
    int powIter0 = 0;
    int powIter1 = 0;

    //std::string timeoutTime;

    const size_t max_samps_per_packet = rx_stream->get_max_num_samps();
    const float burst_pkt_time = std::max(0.100, (2 * max_samps_per_packet/10e6));
    float timeout = burst_pkt_time + INIT_DELAY;

    uhd::rx_metadata_t md;


    std::string pTime0;
    std::string pTime1;
    int num_rx_samps2 = total_num_samps/10;
    int bufferloop_count = 0;

    while (true) {

        //boost::system_time now = boost::get_system_time();
        //timeoutTime = computeTime();

        size_t num_rx_samps = rx_stream->recv(buff_ptrs, total_num_samps, md, timeout);

        //overall_counter++;
        //process_counter++;

        calcPSD(buffs[0], psd_average0, num_rx_samps2, FTsize, bufferloop_count);
        calcPSD(buffs[1], psd_average1, num_rx_samps2, FTsize, bufferloop_count);

        //calcPowerProfile(buffs[0], pow_time_avg0, num_rx_samps2, bufferloop_count);
        //calcPowerProfile(buffs[1], pow_time_avg1, num_rx_samps2, bufferloop_count);


        if(bufferloop_count == bufferloop_to_average-1)
        {
            bufferloop_count = 0;


            doPsdDetection(bufferloop_to_average, FTsize, psd_average0, nLoopsAbove0, psd0, 0, pow_time_mean0);
            doPsdDetection(bufferloop_to_average, FTsize, psd_average1, nLoopsAbove1, psd1, 1, pow_time_mean1);

            //doPowerDetection(pow_time_mean0, pow_time_avg0, bufferloop_to_average);
            //doPowerDetection(pow_time_mean1, pow_time_avg1, bufferloop_to_average);



            int nThresholdAbove = 1;

            // L1
            float threshold0 = 25;

            powProfile0[powIter0] = pow_time_mean0;

            if(powIter0 == 0)
            {
                nPacketsAbove0 = 0;
            }

            if(pow_time_mean0 > threshold0)
            {
                nPacketsAbove0++;
            }

            if(powIter0 == powIterator-1)
            {

                if(nPacketsAbove0 >= nThresholdAbove)
                {

                    if (!bLast0)
                    {
                        pTime0 = computeTime();


                        P_MAX0 = 0.0;
                    }


                    if(write_to =="File")
                    {

                        writePowProfileToFile(0, powProfile0, powIterator, pTime0);

                    }//end of if(write_to =="File")

                    else{
                        std::string DS_id, DS_time;
                        if (nPacketsAbove0 == 1)
                            boost::tie(DS_id, DS_time) = createDatastream("Fraunhofer", "Power Profile", 0);
                        SendToServer(powProfile0, "Fraunhofer", DS_id, DS_time);
                    }


                    if (pow_time_mean0 > P_MAX0)
                    {
                        P_MAX0 = pow_time_mean0;

                        // log PSD
                        if(write_to == "File")
                            writePSDdetectionToFile(0, FTsize, psd0, pTime0);
                        else{
                            std::string DS_id, DS_time;
                            if (!bLast0)
                                boost::tie(DS_id, DS_time) = createDatastream("Fraunhofer", "Power Spectrum", 0);

                            SendToServer(psd0, "Fraunhofer", DS_id, DS_time);
                        }

                        // log spectogram
                        calcSpectrogram(buffs[0], STFTsize, spectrogram_loops, 0, spectrogram0);

                        if(write_to =="File")
                            writeSpectrogramToFile(0, STFTsize, spectrogram_loops, spectrogram0, pTime0);
                        else{
                            std::string DS_id, DS_time;

                            if (!bLast0)
                                boost::tie(DS_id, DS_time) = createDatastream("Fraunhofer", "Spectrogram", 0);

                            SendToServer(spectrogram0, "Fraunhofer", DS_id, DS_time);
                        }

                    }
                    bLast0 = true;
                }
                else
                {
                    bLast0 = false;
                }

                powIter0 = 0;

            }//end of if(nLoopsAbove0 >= 1)
            else
            {
                powIter0++;
            }
            // second band
            float threshold1 = 100;

            powProfile1[powIter1] = pow_time_mean1;

            if(powIter1 == 0)
            {
                nPacketsAbove1 = 0;
            }


            if(pow_time_mean1 > threshold1)
            {
                nPacketsAbove1++;
            }

            if(powIter1 == powIterator-1)
            {

                if(nPacketsAbove1 >= nThresholdAbove)
                {

                    if (!bLast1)
                    {
                        pTime1 = computeTime();

                        P_MAX1 = 0.0;
                    }
                    // Log power profile

                    if(write_to =="File")
                    {

                        writePowProfileToFile(1, powProfile1, powIterator, pTime1);

                    } // End of if(write_to =="File")

                    else{
                        std::string DS_id, DS_time;
                        if (!bLast1)
                            boost::tie(DS_id, DS_time) = createDatastream("Fraunhofer", "Power Profile", 1);

                        SendToServer(powProfile1, "Fraunhofer", DS_id, DS_time);
                    }
                    if (pow_time_mean1 > P_MAX1)
                    {
                        P_MAX1 = pow_time_mean1;

                        // log PSD
                        if(write_to == "File")
                            writePSDdetectionToFile(1, FTsize, psd1, pTime1);
                        else{
                            std::string DS_id, DS_time;

                            if (!bLast1)
                                boost::tie(DS_id, DS_time) = createDatastream("Fraunhofer", "Power Spectrum", 1);

                            SendToServer(psd1, "Fraunhofer", DS_id, DS_time);
                        }


                        // log spectogram
                        calcSpectrogram(buffs[1], STFTsize, spectrogram_loops, 1, spectrogram1);

                        if(write_to =="File")
                            writeSpectrogramToFile(1, STFTsize, spectrogram_loops, spectrogram1, pTime1);
                        else{
                            std::string DS_id, DS_time;

                            if (!bLast1)
                                boost::tie(DS_id, DS_time) = createDatastream("Fraunhofer", "Spectrogram", 1);

                            SendToServer(spectrogram1, "Fraunhofer", DS_id, DS_time);
                        }
                    }

                    bLast1 = true;
                }
                else
                {
                    bLast1 = false;
                }

                powIter1 = 0;
            }
            else
            {
                powIter1++;
            }
        }
        else
        {
            bufferloop_count++;
        }

        num_samps += num_rx_samps;

        //handle the error codes
        if (md.error_code == uhd::rx_metadata_t::ERROR_CODE_TIMEOUT){
            std::cout << boost::format("Timeout while streaming") << std::endl;

            //recvTask(usrp, rx_stream, total_num_samps, bufferloop_to_average, rate, write_to);

            break;
        }
        if (md.error_code == uhd::rx_metadata_t::ERROR_CODE_OVERFLOW){

            last_time = md.time_spec;
            if (overflow_message) {
                overflow_message = false;
                std::cerr << boost::format(
                        "Got an overflow indication. Please consider the following:\n"
                        "  Your write medium must sustain a rate of %fMB/s.\n"
                        "  Dropped samples will not be written to the file.\n"
                        "  Please modify this example for your purposes.\n"
                        "  This message will not appear again.\n"
                ) % (usrp->get_rx_rate()*sizeof(float)/1e6);
            }
            continue;
        }
        if (md.error_code != uhd::rx_metadata_t::ERROR_CODE_NONE) {
            dropped_samps += (md.time_spec - last_time).to_ticks(rate);
            throw std::runtime_error(
                    str(boost::format("Receiver error %s") % md.strerror()));
            //std::cout<<"UHD Error code: None"<<std::endl;
        }


        //fpmetadata <<process_counter << "\t" <<overall_counter <<std::endl;

    }//End of while

    rx_stream->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS);


    //fpmetadata.close();
} // end of processChannel

/***********************************************************************
 * Main code + dispatcher
 **********************************************************************/
int UHD_SAFE_MAIN(int argc, char *argv[]){
    uhd::set_thread_priority_safe();

    //variables to be set by po
    std::string args, sync, subdev, channel_list, file, rx_channel_list, ant, ref, write_mode;
    //double seconds_in_future;

    double spb;
    double rate0, rate1, freq0, freq1, gain0, gain1;
    //bool condTimeout = false;
    std::string filename, timeout;

    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
                    ("help", "help message")

                    ("args",po::value<std::string>(&args)->default_value("addr=192.168.30.2, second_addr=192.168.40.2"),"single uhd device address args") //192.168.10.2
                    //("secs", po::value<double>(&seconds_in_future)->default_value(1.5),"number of seconds in the future to receive")
                    //("nsamps", po::value<size_t>(&total_num_samps)->default_value(250000), "total number of samples to receive")
                    ("spb", po::value<double>(&spb)->default_value(10e6), "samples per buffer")

                    ("rate0", po::value<double>(&rate0)->default_value(100e6),"rate of incoming samples")
                    ("rate1",po::value<double>(&rate1)->default_value(100e6),"rate of incoming samples")
                    ("freq0",po::value<double>(&freq0)->default_value(1560e6),"RF center frequency in Hz - 1575.42e6 Hz")
                    ("freq1",po::value<double>(&freq1)->default_value(1210e6),"RF center frequency in Hz - L5-1176.45e6 Hz")//L2 - 1227.60e6
                    ("gain0",po::value<double>(&gain0)->default_value(0),"gain for the RF chain")
                    ("gain1",po::value<double>(&gain1)->default_value(0),"gain for the RF chain")
                    ("ant", po::value<std::string>(&ant)->default_value("TX/RX"),"antenna selection")

                    ("write to",po::value<std::string>(&write_mode)->default_value("Fraunhofer"),"write to File, Fraunhofer or SensorUp")
                    ("ref", po::value<std::string>(&ref), "clock reference (internal, external, mimo, gpsdo)")
                    ("sync", po::value<std::string>(&sync)->default_value("now"),"synchronization method: now, pps, mimo")
                    ("subdev", po::value<std::string>(&subdev)->default_value("A:0 B:0"),"subdev spec (homogeneous across motherboards)")
                    ("channels",po::value<std::string>(&channel_list)->default_value("0,1"), "which channel(s) to use (specify \"0\", \"1\", \"0,1\", etc)")
                    //("rx_channels", po::value<std::string>(&rx_channel_list)->default_value("0,1"), "which RX channel(s) to use (specify \"0\", \"1\", \"0,1\", etc)")
                    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc, po::command_line_style::default_style), vm);
    po::notify(vm);


    int duration_to_average = 1;

    int bufferloops_to_average = (duration_to_average * rate0) / spb;

    if (vm.count("help")){
        std::cout << boost::format("UHD RX Multi Samples %s") % desc << std::endl;
        std::cout <<
                "    This is a demonstration of how to receive data from multiple channels.\n"
                "    This example can receive from multiple RX daughterboards.\n"
                "\n"
                "    Specify --subdev to select multiple channels.\n"
                "      Ex: --subdev=\"0:A 0:B\" to get 2 channels on a Basic RX.\n"
                "\n"
                "    Specify --args to select multiple motherboards in a configuration.\n"
                "      Ex: --args=\"addr0=192.168.10.2, addr1=192.168.10.3\"\n"
                << std::endl;
        return ~0;
    }

    //create a usrp device
    std::cout << std::endl;
    uhd::device_addrs_t device_addrs = uhd::device::find(args, uhd::device::USRP);
    if (not device_addrs.empty() and device_addrs.at(0).get("type", "") == "usrp1"){
        std::cerr << "*** Warning! ***" << std::endl;
        std::cerr << "Benchmark results will be inaccurate on USRP1 due to insufficient features.\n" << std::endl;
    }
    std::cout << boost::format("Creating the usrp device with: %s...") % args << std::endl;
    uhd::usrp::multi_usrp::sptr usrp = uhd::usrp::multi_usrp::make(args);

    //always select the subdevice first, the channel mapping affects the other settings
    if (vm.count("subdev"))
        usrp->set_rx_subdev_spec(subdev); //sets across all mboards

    std::cout << boost::format("Using Device: %s") % usrp->get_pp_string() << std::endl;
    int num_mboards = usrp->get_num_mboards();

    boost::thread_group detectorThread;
    //boost::thread *detectorThread = new boost::thread();

    if(vm.count("ref"))
    {
        if (ref == "mimo")
        {
            if (num_mboards != 2) {
                std::cerr << "ERROR: ref = \"mimo\" implies 2 motherboards; your system has " << num_mboards << " boards" << std::endl;
                return -1;
            }
            usrp->set_clock_source("mimo",1);
        } else {
            usrp->set_clock_source(ref);
        }

        if(ref != "internal") {
            std::cout << "Now confirming lock on clock signals..." << std::endl;
            bool is_locked = false;
            boost::system_time end_time = boost::get_system_time() + boost::posix_time::milliseconds(CLOCK_TIMEOUT);
            for (int i = 0; i < num_mboards; i++) {
                if (ref == "mimo" and i == 0) continue;
                while((is_locked = usrp->get_mboard_sensor("ref_locked",i).to_bool()) == false and
                        boost::get_system_time() < end_time )
                {
                    boost::this_thread::sleep(boost::posix_time::milliseconds(1));
                }
                if (is_locked == false) {
                    std::cerr << "ERROR: Unable to confirm clock signal locked on board:" << i <<  std::endl;
                    return -1;
                }
                is_locked = false;
            }
        }
    }


    //check that the device has sufficient RX and TX channels available
    std::vector<std::string> channel_strings;
    std::vector<size_t> rx_channel_nums;
    if (vm.count("rx_rate")) {
        if (!vm.count("rx_channels")) {
            rx_channel_list = channel_list;
        }

        boost::split(channel_strings, rx_channel_list, boost::is_any_of("\"',"));
        for (size_t ch = 0; ch < channel_strings.size(); ch++) {
            size_t chan = boost::lexical_cast<int>(channel_strings[ch]);
            if (chan >= usrp->get_rx_num_channels()) {
                throw std::runtime_error("Invalid channel(s) specified.");
            } else {
                rx_channel_nums.push_back(boost::lexical_cast<int>(channel_strings[ch]));
            }
        }
    }


    std::cout << boost::format("Setting device timestamp to 0...") << std::endl;

    //set the rx sample rate (sets across all channels)
    std::cout<< boost::format("Setting RX Rate - channel0: %f Msps...")   % (rate0 / 1e6) << std::endl;
    usrp->set_rx_rate(rate0, 0);
    std::cout<< boost::format("Actual RX Rate-channel0: %f Msps...")% (usrp->get_rx_rate(0) / 1e6) << std::endl << std::endl;

    std::cout<< boost::format("Setting RX Rate - channel1: %f Msps...")   % (rate1 / 1e6) << std::endl;
    usrp->set_rx_rate(rate1, 1);
    std::cout<< boost::format("Actual RX Rate-channel1: %f Msps...")% (usrp->get_rx_rate(1) / 1e6) << std::endl << std::endl;

    //set the center frequency (sets across all channels)
    std::cout<< boost::format("Setting RX Freq for channel 0: %f MHz...")% (freq0 / 1e6) << std::endl;
    uhd::tune_request_t tune_request0(freq0);
    tune_request0.args = uhd::device_addr_t("mode_n=integer");
    usrp->set_rx_freq(tune_request0, 0);
    std::cout<< boost::format("Actual RX Freq for channel 0: %f MHz...")% (usrp->get_rx_freq(0) / 1e6) << std::endl << std::endl;

    std::cout<< boost::format("Setting RX Freq for channel1: %f MHz...")% (freq1 / 1e6) << std::endl;
    uhd::tune_request_t tune_request1(freq1);
    tune_request1.args = uhd::device_addr_t("mode_n=integer");
    usrp->set_rx_freq(tune_request1, 1);
    std::cout<< boost::format("Actual RX Freq for channel 1: %f MHz...")% (usrp->get_rx_freq(1) / 1e6) << std::endl << std::endl;

    //set the gain (sets across all channels)
    std::cout<< boost::format("Setting RX Gain for channel 0: %f dB...") % gain0<< std::endl;
    usrp->set_rx_gain(gain0, 0);
    std::cout<< boost::format("Actual RX Gain for channel 0: %f dB...")   % usrp->get_rx_gain(0) << std::endl << std::endl;

    std::cout<< boost::format("Setting RX Gain for channel 1: %f dB...") % gain1<< std::endl;
    usrp->set_rx_gain(gain1, 1);
    std::cout<< boost::format("Actual RX Gain for channel 1: %f dB...")   % usrp->get_rx_gain(1) << std::endl << std::endl;

    //set antenna
    usrp->set_rx_antenna(ant, 0);
    usrp->set_rx_antenna(ant, 1);

    std::vector<size_t> channel_nums;
    channel_nums.push_back(0);
    channel_nums.push_back(1);
    uhd::stream_args_t stream_args("fc32", "sc16"); //complex floats
    stream_args.channels = channel_nums;
    uhd::rx_streamer::sptr rx_stream = usrp->get_rx_stream(stream_args);

    //sleep for the required duration
    boost::this_thread::sleep(boost::posix_time::milliseconds(100));

    while (true) {

        recvTask(usrp, rx_stream, spb, bufferloops_to_average, rate0, write_mode);

        //Logging timeout occurance
        timeout = computeTime();
        filename = "/home/navcert/workspace/ProcessChannel/TimeoutData";
        fptimeoutData.open(filename.c_str(), std::ios_base::out | std::ios_base::app);
        fptimeoutData << timeout << std::endl;
        fptimeoutData.flush();
        fptimeoutData.close();

    }

    //finished
    std::cout << std::endl << "Done!" << std::endl << std::endl;
    return EXIT_SUCCESS;
}
