#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using namespace std;


#define SPECTIS1_BAUDRATE 115200
#define SPECTIS1_DATASIZE 8

class Serial {

public:
  Serial(boost::asio::serial_port & port) : m_port(port),
					    m_timer(m_port.get_io_service()),
					    m_keep_running(true), m_linesize(0) {
  }

  void start(){
    boost::asio::deadline_timer timer(m_port.get_io_service());
    while(m_keep_running){
      read_serial();
      m_timer.expires_from_now(boost::posix_time::milliseconds(500));
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
  char c;
  int m_linesize;



  void read_complete(const boost::system::error_code& error, size_t bytes_transferred){
    m_linesize++;
    std::cout << c << " 0x" << std::hex << (0xff & int(c)) << std::dec << " ";
    if(m_linesize>=16){
      std::cout << std::endl;
      m_linesize = 0;
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
    c = '0';
    m_port.get_io_service().reset();
		  
    boost::asio::async_read(m_port,
			    boost::asio::buffer(&c, 1),
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
