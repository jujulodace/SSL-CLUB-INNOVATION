/*
    This file is part of SSL.

    Copyright 2018 Boussicault Adrien (adrien.boussicault@u-bordeaux.fr)

    SSL is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SSL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with SSL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "logger.h"
#include <iostream>
#include <sstream>
#include <debug.h>

Logger::Logger(): active(false), plot(new Plot()) { } 

void Logger::init_plot(
    const std::string & name, const std::vector<std::string> & value_names 
){
    plot->init( name, value_names );
}
void Logger::init_file( const std::string & file_name ){
    log_file.open( file_name );
    if( ! log_file.is_open() ){
        std::cerr << "ERROR : It is not possible to write in " << file_name << std::endl;
        return ;
    }
}

void Logger::log_plot( const std::string & name, double value ){
    if( active ){
        if( plot ){
            plot->log( name, value);
        }
    }
}

void Logger::log_plot( std::function< std::vector<double>() > fct ){
    if( active ){
        if( plot ){
            plot->log( fct );
        }
    }
}

void Logger::store_plot(){
    if( plot ){
        plot->store();
    }
}

bool Logger::is_active() const {
    return active;
}

void Logger::log( const std::string & message ){
    if( is_active() ){
        if( log_file.is_open() ){
            log_file << message;
        }
    }
}

void Logger::print( const std::string & message ) const {
    if( is_active() ){
        std::cout << message;
    }
}

void Logger::activate(){
    active = true;
}

void Logger::desactivate(){
    active = false;
}

Plot* Logger::get_plot(){
    return plot;
}

Logger::~Logger(){
    delete plot;
}
