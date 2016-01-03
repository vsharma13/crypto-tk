//
// libsse_crypto - An abstraction layer for high level cryptographic features.
// Copyright (C) 2015 Raphael Bost
//
// This file is part of libsse_crypto.
//
// libsse_crypto is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// libsse_crypto is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with libsse_crypto.  If not, see <http://www.gnu.org/licenses/>.
//

#include "fpe.hpp"
#include "aez/aez.h"
#include "random.hpp"

#include <cstring>
#include <exception>
#include <iostream>
#include <iomanip>


namespace sse
{

namespace crypto
{

class Fpe::FpeImpl
{
public:
		

	FpeImpl();
	
	FpeImpl(const std::array<uint8_t,kKeySize>& k);
	
	// ~FpeImpl();

	void encrypt(const unsigned char* in, const size_t &len, unsigned char* out);
	void encrypt(const std::string &in, std::string &out);
	void decrypt(const unsigned char* in, const size_t &len, unsigned char* out);
	void decrypt(const std::string &in, std::string &out);


private:
	void setup(const unsigned char* k);
		
	aez_ctx_t aez_ctx_;
	
};

Fpe::Fpe() : fpe_imp_(new FpeImpl())
{
	
}
	
Fpe::Fpe(const std::array<uint8_t,kKeySize>& k) : fpe_imp_(new FpeImpl(k))
{	
}

Fpe::~Fpe() 
{ 
	delete fpe_imp_;
}

void Fpe::encrypt(const std::string &in, std::string &out)
{
	fpe_imp_->encrypt(in, out);
}
void Fpe::decrypt(const std::string &in, std::string &out)
{
	fpe_imp_->decrypt(in, out);
}
	
Fpe::FpeImpl::FpeImpl()
{
	unsigned char k[kKeySize];
	random_bytes(kKeySize, k);
	setup((unsigned char*)k);
}

Fpe::FpeImpl::FpeImpl(const std::array<uint8_t,kKeySize>& k)
{	
	setup((unsigned char*)k.data());
}

void Fpe::FpeImpl::setup(const unsigned char* k)
{
	aez_setup(k, 48, &aez_ctx_);
}
	
void Fpe::FpeImpl::encrypt(const unsigned char* in, const size_t &len, unsigned char* out)
{
	char iv[16] = {0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00};
	aez_encrypt(&aez_ctx_, iv, 16,
	                 NULL, 0, 0,
	                 (char *)in, len, (char *)out);
}

void Fpe::FpeImpl::encrypt(const std::string &in, std::string &out)
{
	unsigned int len = in.size();
	out.resize(len);
	encrypt((unsigned char*)in.data(), len, (unsigned char*)out.data());
}

void Fpe::FpeImpl::decrypt(const unsigned char* in, const size_t &len,  unsigned char* out)
{
	char iv[16] = {0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00};
	aez_decrypt(&aez_ctx_, iv, 16,
	                 NULL, 0, 0,
	                 (char *)in, len, (char *)out);
}

void Fpe::FpeImpl::decrypt(const std::string &in, std::string &out)
{
	unsigned int len = in.size();
	out.resize(len);
	decrypt((unsigned char*)in.data(), len, (unsigned char*)out.data());
}

	
}
}