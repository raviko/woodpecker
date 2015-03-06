#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/time_facet.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;


#define SPECTIS1_BAUDRATE 115200
#define SPECTIS1_DATASIZE 8

class Serial {

public:
  Serial(boost::asio::serial_port & port) : m_port(port),
					    m_timer(m_port.get_io_service()),
					    m_keep_running(true) {
  }

  void start(){
    boost::asio::deadline_timer timer(m_port.get_io_service());
    while(m_keep_running){
      read_serial();
      m_timer.expires_from_now(boost::posix_time::milliseconds(100));
      m_timer.async_wait(boost::bind(&Serial::time_out, this,
				   boost::asio::placeholders::error));
		  
      m_port.get_io_service().run();
    }
  }

  void stop(){
    m_keep_running = false;
  }
  
private:
  boost::asio::serial_port & m_port;
  boost::asio::deadline_timer m_timer;
  bool m_keep_running;
  boost::asio::streambuf  buf;



  void read_complete(const boost::system::error_code& error, size_t bytes_transferred){
    boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();

    if( bytes_transferred > 0 )
{
  
      std::istream ss(&buf);
      std::string s;

      boost::posix_time::time_facet *facet = new boost::posix_time::time_facet("%H:%M:%S.%f");
      std::cout.imbue(locale(std::cout.getloc(), facet));
      ss >> s;
      for(std::string::iterator i=s.begin(); i!=s.end(); ++i){

	std::cout <<  now
	  <<" | "
	  << *i << " 0x" << std::hex << (0xff & *i) << std::dec << std::endl;
	
      }

    }
      m_timer.cancel();    
  }

  void time_out(const boost::system::error_code& error) {
 
    if (error) {
      return;
    }
 
    m_port.cancel();
  }


  bool read_serial(){
    m_port.get_io_service().reset();
		  
    boost::asio::async_read(m_port,
			    buf,
			    boost::bind(&Serial::read_complete, this, 
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		  
    return true;
  }


 

};


int main(int argc, char* argv[]){

  if(argc != 2){
    std::cerr << "Wrong number of arguments: " << argc << std::endl;
    return EXIT_FAILURE;
  }
  boost::asio::io_service io;
  boost::asio::serial_port serial(io);
  boost::system::error_code er;

  try {
    serial.open(argv[1], er);

    if(er){
      std::cerr << "Device open failed: " << argv[1] << std::endl;
      return EXIT_FAILURE;
    }

    std::cerr << "Device connected to: " << argv[1] << std::endl;

    
    serial.set_option(boost::asio::serial_port::baud_rate(SPECTIS1_BAUDRATE));
    serial.set_option(boost::asio::serial_port::character_size(SPECTIS1_DATASIZE));
    serial.set_option(boost::asio::serial_port::parity(boost::asio::serial_port::parity::none));
    serial.set_option(boost::asio::serial_port::stop_bits(boost::asio::serial_port::stop_bits::one));
    serial.set_option(boost::asio::serial_port::flow_control(boost::asio::serial_port::flow_control::none));


    Serial s(serial);
    s.start();

    serial.close();
		
  }
  catch(boost::system::system_error e){
    std::cerr << "Conection failed for " << argv[1] << " " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  
  
}
