// Copyright (c) 2015 The Sibcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef OFFERDB_H
#define	OFFERDB_H

#include "wallet/db.h"

#include <string>

/** Access to the offer database (sib.dat) */

// TODO: use SQLite engine
class COfferDB : public CDB
{
public:
    COfferDB(const std::string& strFilename, const char* pszMode = "cr+") : CDB(strFilename, pszMode)
    {
    }

    bool WriteName(const std::string& key, const std::string &arr);
    bool ReadName(const std::string& key,  std::string &arr);
    bool EraseName(const std::string& key);
    
};
#endif	/* SIBDB_H */

