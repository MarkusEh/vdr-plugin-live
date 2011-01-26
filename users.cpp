/*
 * users.cpp: User specific rights for the LIVE plugin.
 *
 * See the README file for copyright information and how to reach the author.
 */

#include "users.h"
#include <iostream>
#include <sstream>
#include "tools.h"
#include "setup.h"

namespace vdrlive {

using namespace std;

std::string cUsers::logged_in_user;

// -- cUser -----------------------------------------------------------------
cUser::cUser(int ID, const std::string& Name, const std::string& Password) 
  : m_ID(ID), m_Name(Name) 
{
	SetPassword(Password);
}

void cUser::SetPassword(const std::string Password) 
{
	ostringstream passwordStr;
	passwordStr << Password.size() << "|" << MD5Hash(Password);
	m_PasswordMD5 = passwordStr.str();
}

int cUser::GetPasswordLength() const
{
	// format is <length>:<md5-hash of password>
	vector< string > parts = StringSplit( m_PasswordMD5, '|' );
	return (parts.size() > 0) ? lexical_cast< int >( parts[0] ) : 0;
}

std::string const cUser::GetMD5HashPassword() const
{
	// format is <length>:<md5-hash of password>
	vector< string > parts = StringSplit( m_PasswordMD5, '|' );
	return (parts.size() > 1) ? parts[1] : "";
}

bool cUser::Parse(const char *s)
{
  char *line;
  char *pos;
  char *pos_next;
  int parameter = 1;
  int valuelen;
#define MAXVALUELEN (10 * MaxFileName)

  char value[MAXVALUELEN];

  pos = line = strdup(s);
  pos_next = pos + strlen(pos);
  if (*pos_next == '\n') *pos_next = 0;
  while (*pos) {
    while (*pos == ' ') pos++;
    if (*pos) {
      if (*pos != ':') {
        pos_next = strchr(pos, ':');
        if (!pos_next)
          pos_next = pos + strlen(pos);
        valuelen = pos_next - pos + 1;
        if (valuelen > MAXVALUELEN) 
		{
			esyslog("entry '%s' is too long. Will be truncated!", pos);  
	    	valuelen = MAXVALUELEN;
		}
        strn0cpy(value, pos, valuelen);
        pos = pos_next;

		switch (parameter) {
		case 1: m_ID = lexical_cast< int >(value);
			break;
		case 2:  m_Name = value;
			break;
		case 3: m_PasswordMD5 = value;
			break;
		case 4: 
			m_Userrights = lexical_cast< int >(value);
			break;
		default:
			break;
        } //switch
      }
      parameter++;
    }
    if (*pos) pos++;
  } //while
  
  free(line);
  return (parameter >= 4) ? true : false;
}

const char *cUser::ToText(void)
{
    char* buffer = NULL;
    if (asprintf(&buffer, "%d:%s:%s:%d", m_ID, m_Name.c_str(), m_PasswordMD5.c_str(), m_Userrights) < 0)
        return NULL;
    return buffer;
}

bool cUser::Save(FILE *f)
{
    return fprintf(f, "%s\n", ToText()) > 0;
}

bool cUser::HasRightTo(eUserRights right) 
{ 
	return ((m_Userrights & (1 << (right-1))) != 0);
}

bool cUser::CurrentUserHasRightTo(eUserRights right) 
{ 
	if (!LiveSetup().UseAuth()) return true;
	cUser* user = cUsers::GetByUserName(cUsers::logged_in_user);
	return (cUsers::logged_in_user == LiveSetup().GetAdminLogin() || (user && (user->m_Userrights & (1 << (right-1))) != 0)); 
}

void cUser::SetRight(eUserRights right)
{
	isyslog("set right '%d' in '%d'", right, m_Userrights);
	m_Userrights |= (1 << (right-1));
	isyslog("now rights are '%d'", m_Userrights);
}

bool cUsers::Delete(const std::string& Name)
{
  cUser* user = Users.First();
  while (user) 
    {
      if (user->Name() == Name)
	{
	  Users.Del(user);
	  Users.Save();
	  return true;
	}
    user = Users.Next(user);
    }
  return false;
}

cUser* cUsers::GetByUserId(const std::string& Id)
{
	cUser* user = Users.First();
	while (user) 
	{
		if (user->Id() == atoi(Id.c_str()))
			return user;
		user = Users.Next(user);
    }
	return NULL;
}

cUser* cUsers::GetByUserName(const std::string& Name)
{
	cUser* user = Users.First();
	while (user) 
	{
		if (user->Name() == Name)
			return user;
		user = Users.Next(user);
    }
	return NULL;
}

int cUsers::GetNewId()
{
	int iMaxId = -1;
	cUser* user = Users.First();
	while (user) 
	{
		if (iMaxId < user->Id())
			iMaxId = user->Id();
		user = Users.Next(user);
    }
	return iMaxId + 1;
}

bool cUsers::ValidUserLogin(const std::string& login, const std::string& password)
{
	cUser* user = GetByUserName(login);
	if (user && MD5Hash(password) == user->GetMD5HashPassword())
		return true;
	return false;
}

bool cUsers::ValidLogin(const std::string& login, const std::string& password)
{
	return ((login == LiveSetup().GetAdminLogin() && MD5Hash(password) == LiveSetup().GetMD5HashAdminPassword()) ||
		ValidUserLogin(login, password));
}

}
