
#include "client.hpp"
// #include "webserv.hpp"
#include "request.hpp"
#include "HttpForms.hpp"
#include "utils.hpp"
#include "cgi_handler.hpp"



#define MAX_HEADER_SIZE 8192  // 8 KB


bool client::handle_jesus(pollfd& pfd)
{
    char buffer[2048];
    std::string response;
    ssize_t bytes_received = 0;
   
    //  1er INIT, lire le premier contenu sur le socket
    if ((pfd.revents & POLLIN) )
    {
		if( this->status == WAITING && this->_request == NULL)
		{
			//    std::cerr<<" aaa "<<pfd.revents << " on pfd" << pfd.fd;
			// std::cerr<<"new req. ";
			this->status = READING;
			this->_request = new Request(*serv, fd, this->status);
			this->status = this->_request->_request_status;
			tryLaunchCGI();
		}
		// if( this->status == WAITING && this->_request)
		// {
		// 	std::cerr<<" dell req:";
		// 	delete this->_request;
		// 	this->_request = new Request(*serv, fd, this->status);
		// 	this->status = this->_request->_request_status;
		// 	tryLaunchCGI();
		// }
    }

    // si tout le header est compris dans le 1er read, le parse, sinon read encore
    if ((pfd.revents & POLLIN))
    {
		// std::cerr<<" STAUTS \n"<<this->status;
		if (this->status == READINGHEADER && this->_request)
		{
			this->_request->readRequest();
			if (this->_request->checkHeaderCompletion())
				this->keepalive = this->_request->keepalive;
			if(this->_request->_request_status == EXECUTING)
			{
				this->keepalive = this->_request->keepalive;
				// std::cerr<<" bexecutes. ";
				this->_request->execute(this->_request->getExecCode());
				this->status = this->_request->_request_status;
				tryLaunchCGI();
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
		if (this->_request->iscgi && this->status != WAITING)
		{
			// std::cerr<<" CCC "<<pfd.revents << " on pfd" << pfd.fd;
			this->status = WAITING;
			// this->cgi_fd = this->_request->launchCGI();
			tryLaunchCGI();
			// std::cerr<<" onfd: "<<this->cgi_fd<<" ";
		}
    }
	
	// std::cerr<<" DDD "<<pfd.revents << " on pfd" << pfd.fd;
	if (this->status == WRITING)
	{
		// std::cerr<<" WRITING.SETTING.POLLOUT. : "<<this->status;
		pfd.events =  POLLOUT;

		
		
	}
	return false;
}

bool client::answerClient(pollfd& pfd)
{
	// std::cerr<<" ANSWER.CLIENT :"<<this->status;
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
		
		// std::cerr<<" DONE.req.DELETE "<<this->status;
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
		// std::cerr << " AAAAAAAAAAAA ";
        this->cgi_fd = this->cgi_handler->launch();//std::cerr << " BBBBBBBBBBBBB: ";
        // std::cerr << "CGI launched on fd: " << this->cgi_fd << std::endl;
		// std::cerr << "getreqbod: " << this->cgi_handler->getrequestBody() << std::endl;

        // Passe le client en état attente CGI
        this->status = WAITING;
    }
}

