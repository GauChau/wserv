/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gchauvot <gchauvot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 14:21:26 by gchauvot          #+#    #+#             */
/*   Updated: 2025/06/02 16:43:19 by gchauvot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "request.hpp"
// #include "webserv.cpp"

static void printlocation(struct LocationConfig loc, std::string msg);


Request::Request(char *raw) : authorized(false)
{
	r_full_request = raw;
	std::istringstream iss(raw);
	// this->_loc =NULL;

	iss>> this->r_method>> this->r_location >> this->r_version;
    // std::cout<<"m: "<<metho<<" p: "<<path<<" v: "<<version<<"\n";
}
Request::Request(char *raw, ServerConfig server) : authorized(false)
{
	r_full_request = raw;
	std::istringstream iss(raw);

	iss>> this->r_method>> this->r_location >> this->r_version;
	check_allowed_methods(server);
}

Request::~Request()
{

}

void Request::check_allowed_methods(ServerConfig server)
{
	std::vector<LocationConfig>::const_iterator it_loc = server.locations.begin();
    for(;it_loc != server.locations.end();it_loc++)
    {
        // std::cout<<"servlocs 2|"<<it_loc->path<< "|"<<std::endl;
		if(it_loc->path == this->r_location)
		{

			this->_loc = *it_loc;
			// printlocation(this->_loc,this->r_location);
			std::vector<std::string>::const_iterator it_meth = it_loc->allowed_methods.begin();
			for(;it_meth != it_loc->allowed_methods.end();it_meth++)
			{
			    // std::cout<<"methloop: |"<<*it_meth<<"|"<<std::endl;
			    if(this->r_method == *it_meth)
			    {
			        // std::cout<<"method |"<<this->r_method<< "|:AUTHORIZED"<<std::endl;
			        this->authorized = true;
					return ;
			    }
			}
			return ;
		}
    }

}

void Request::execute()
{
	std::cout<<"At location:|" << this->r_location<<"| <";
	if(!this->authorized)
		std::cout<<"method "<<this->r_method<< ": NOT AUTHORIZED>"<<std::endl;
	else if (this->r_method == "GET")
		this->Get();
	else if (this->r_method == "POST")
		this->Post();

}

void Request::Post()
{
	std::cout<<"POST | EXECUTED !!>"<<std::endl;
	// this->authorized =false;
}
void Request::Get()
{
	std::cout<<"GET | EXECUTED !!>"<<std::endl;
	// this->authorized =false;
}