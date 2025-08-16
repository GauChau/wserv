#include "client.hpp"
// #include "webserv.hpp"
#include "request.hpp"
#include "HttpForms.hpp"
#include "utils.hpp"



#define MAX_HEADER_SIZE 8192  // 8 KB

void client::test(pollfd& pfd)
{
	char buffer[2048];
    std::string response;
    ssize_t bytes_received = 0;
	// std::cerr << "hand, ";

	    if ((pfd.revents & POLLIN) )
    {
		if( this->status == WAITING && this->_request == NULL)
       {
		 this->status = READING;
        this->_request = new Request(*serv, fd, this->status);
		}
        // return false;
    }



	if ((pfd.revents & POLLIN) )
    {
		if (this->status == READINGHEADER && this->_request)
		{
			this->_request->readRequest();
			this->_request->checkHeaderCompletion();
			if(this->_request->_request_status == EXECUTING)
			{
				this->_request->execute(this->_request->getExecCode());
				pfd.events = POLLIN & POLLOUT;
			}
		}
    }
};

bool client::handle_jesus(pollfd& pfd)
{
    char buffer[2048];
    std::string response;
    ssize_t bytes_received = 0;
	// std::cerr<<pfd.fd << " hand, " << pfd.revents<<std::endl;
   
    //  1er INIT, lire le premier contenu sur le socket
    if ((pfd.revents & POLLIN) )
    {
		if( this->status == WAITING && this->_request == NULL)
       {
		// std::cerr<<"new req. ";
		 this->status = READING;
        this->_request = new Request(*serv, fd, this->status);
		this->status = this->_request->_request_status;
		// std::cerr<<"req stat: "<<this->status;
		}
	// 	else if( this->status == WAITING && this->_request)
    //    {
	// 	// std::cerr<<"new req. ";
	// 	this->status = READING;
	// 	delete this->_request;
    //     this->_request = new Request(*serv, fd, this->status);
	// 	this->status = this->_request->_request_status;
	// 	// std::cerr<<"req stat: "<<this->status;
	// 	}
        // return false;
    }

    // si tout le header est compris dans le 1er read, le parse, sinon read encore
    if ((pfd.revents & POLLIN))
    {
		// std::cerr<<" Aexecutes: \n"<<this->status;
		std::cerr<<this->_request->getDataRec()+"\n";
		if (this->status == READINGHEADER && this->_request)
		{
			this->_request->readRequest();
			if (this->_request->checkHeaderCompletion())
				this->keepalive = this->_request->keepalive;
			if(this->_request->_request_status == EXECUTING)
			{
				
				// std::cerr<<" bexecutes. ";
				this->_request->execute(this->_request->getExecCode());
				this->status = this->_request->_request_status;
			}
		}
		else if (this->status == READINGDATA && this->_request)
		{
			// std::cerr<<" NEVERHERE: "<<this->status;
			//POST method specific:
			//we knoW HEADER is done, so now just read the socket until all data is secured
			this->_request->readRequest();
			this->_request->checkPostDataOk();
			if(this->_request->_request_status == EXECUTING)
			{
				//all is received, execute
				this->_request->Post_data_write();
			}
			this->status = this->_request->_request_status;
		}
    }
	

	if (this->status == WRITING)
	{
		// std::cerr<<" WRITING.SETTING.POLLOUT. : "<<this->status;
		pfd.events =  POLLOUT;

		// std::cerr<<" \n http respoinse: \n"<<this->_request->_get_ReqContent();
		
	}
	return false;
}

bool client::answerClient(pollfd& pfd)
{
	// std::cerr<<" ANSWER.CLIENT : "<<this->status;
	if((pfd.revents & POLLOUT) && this->status == WRITING && this->_request)
	{	
		// std::cerr<<" SENDING.CLIENT \n"<<this->status;	
		this->_request->sendResponse();
	}
	if (this->_request->_request_status == DONE)
	{
		pfd.events = POLLIN;
		this->status = WAITING;
		this->keepalive = this->_request->keepalive;
		delete this->_request;
		this->_request = NULL;
		return true;
	}
	return false;
}

