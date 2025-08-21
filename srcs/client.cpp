
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
		if( this->status == WAITING && this->_request == NULL)
		{
			std::cerr<<" AAA;  ";
			this->status = READING;
			this->_request = new Request(*serv, fd, this->status);
			this->status = this->_request->_request_status;
			tryLaunchCGI();
			// if (this->status == WRITING)
			// 	pfd.events =  POLLOUT;
			// return true;
		}
    // }

    // // si tout le header est compris dans le 1er read, le parse, sinon read encore
    // if ((pfd.revents & POLLIN))
    // {
		if (this->status == READINGHEADER && this->_request)
		{
			std::cerr<<" BBB;  ";
			this->_request->readRequest();
			if (this->_request->checkHeaderCompletion())
				this->keepalive = this->_request->keepalive;
			if(this->_request->_request_status == EXECUTING)
			{
				this->keepalive = this->_request->keepalive;
				this->_request->execute();
				this->status = this->_request->_request_status;
				tryLaunchCGI();
			}
		}
		else if (this->status == READINGDATA && this->_request)
		{
			std::cerr<<" CCC;  ";
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
		if (this->status == EXECUTING && this->_request)
		{
			std::cerr<<" DDD;  ";
			this->_request->execute();
			this->status = this->_request->_request_status;
			tryLaunchCGI();
		}
		if (this->_request->iscgi && this->status != WAITING)
		{
			this->status = WAITING;
			tryLaunchCGI();
		}
    }
	// std::cerr<<" lient STAT: " << status;
	if (this->status == WRITING)
		pfd.events =  POLLOUT;
	return false;
}

bool client::answerClient(pollfd& pfd)
{
	if((pfd.revents & POLLOUT) && this->status == WRITING && this->_request)
	{	
		std::cerr<<"\nsend : " ;//<< this->_request->_ReqContent;
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

