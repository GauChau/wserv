
#include "client.hpp"
// #include "webserv.hpp"
#include "request.hpp"
#include "HttpForms.hpp"
#include "utils.hpp"
#include "cgi_handler.hpp"



#define MAX_HEADER_SIZE 8192  // 8 KB


bool client::handle_jesus(pollfd& pfd)
{
    if ((pfd.revents & POLLIN) )
    {
		if( this->status == WAITING)
			// std::cerr<<" aaa "<<pfd.revents << " on pfd" << pfd.fd << "this->request: " << this->_request << std::endl;
		if( this->status == WAITING && this->_request == NULL)
		{
			// std::cerr<<"new req. ";
			this->status = READING;
			this->_request = new Request(*serv, fd, this->status);
			this->status = this->_request->_request_status;
			tryLaunchCGI();
			std::cerr<<" 1* ";
		}
    }

    // si tout le header est compris dans le 1er read, le parse, sinon read encore
    if ((pfd.revents & POLLIN))
    {
		// std::cerr<<" bbb "<<pfd.revents << " on pfd" << pfd.fd;
		// std::cerr<<" STAUTS "<<this->status;
		if (this->status == READINGHEADER && this->_request)
		{
			this->_request->readRequest();
			if (this->_request->checkHeaderCompletion())
				this->keepalive = this->_request->keepalive;
			if(this->_request->_request_status == EXECUTING)
			{
				this->keepalive = this->_request->keepalive;
				this->_request->execute();
				this->status = this->_request->_request_status;
				tryLaunchCGI();
				std::cerr<<" 2* ";
			}
		}
		else if (this->status == READINGDATA && this->_request)
		{
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
		if (this->_request->iscgi && this->status != WAITING)
		{
			this->status = WAITING;
			tryLaunchCGI();
			std::cerr<<" 3* ";
		}
    }
	
	if (this->status == WRITING)
	{
		std::cerr<<" ccc "<<pfd.revents << " on pfd" << pfd.fd;
		pfd.events =  POLLOUT;

		
		
	}
	return false;
}

bool client::answerClient(pollfd& pfd)
{
	if((pfd.revents & POLLOUT) && this->status == WRITING && this->_request)
	{	
		this->_request->sendResponse();
	}
	if (this->_request->_request_status == DONE)
	{
		std::cerr<<"  answeer dne ";
		std::cerr<<"\n a* " << this->_request;
		pfd.events = POLLIN;
		this->status = WAITING;
		this->keepalive = this->_request->keepalive;
		std::cerr<<" 1* " << this->_request;
		delete this->_request;
		std::cerr<<" 2* " << this->_request;
		this->_request = NULL;
		std::cerr<<" 3* " << this->_request << std::endl;
		return true;
	}
	return false;
}

void client::tryLaunchCGI()
{
    if (this->_request && this->_request->iscgi && this->cgi_fd == -1)
    {
        // Crée le handler CGI si besoin
        if (!this->cgi_handler)
            this->cgi_handler = new CGIHandler(this);

        this->cgi_handler->setEnv(this->_request->env);
        this->cgi_handler->setScriptPath(this->_request->getScriptPath());
        this->cgi_handler->setRequestBody(this->_request->getRBody());

        // Lance le CGI et récupère le fd
        this->cgi_fd = this->cgi_handler->launch();

        // Passe le client en état attente CGI
        this->status = WAITING;
		
    }
}

client::~client()
{
	if(this->_request != NULL)
		delete this->_request;
	std::cerr<<"client destroyed:"<<this->fd<<std::endl;
	if (cgi_handler)
		delete cgi_handler;
};

