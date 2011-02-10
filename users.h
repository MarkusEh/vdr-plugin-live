#ifndef VDR_LIVE_USERS_H
#define VDR_LIVE_USERS_H

#include <string>
#include <vdr/plugin.h>
#include <vdr/tools.h>
#include <iostream>
#include <sstream>

namespace vdrlive {

enum eUserRights
{
	UR_EDITSETUP=1,
	UR_EDITTIMERS,
	UR_DELTIMERS,
	UR_DELRECS,
	UR_USEREMOTE,
	UR_STARTREPLAY,
	UR_SWITCHCHNL,
	UR_EDITSTIMERS,
	UR_DELSTIMERS,
	UR_EDITRECS
};

// --- cUser --------------------------------------------------------
class cUser : public cListObject {
	int m_ID;
    std::string m_Name;
    std::string m_PasswordMD5;
	int m_Userrights;
public:
	cUser() : m_ID(-1), m_Userrights(0) {}
	cUser(int ID, const std::string& Name, const std::string& Password);
	int Id() const { return m_ID; }
	std::string Name() const { return m_Name; }
	std::string PasswordMD5() const { return m_PasswordMD5; } 
	int Userrights() const { return m_Userrights; }
	int GetPasswordLength() const;
	std::string const GetMD5HashPassword() const;
	void SetId(int Id) { m_ID = Id; }
	void SetName(const std::string Name) { m_Name = Name; }
	void SetPassword(const std::string Password);
	void SetUserrights(int Userrights) { m_Userrights = Userrights; }
	bool HasRightTo(eUserRights right);
	static bool CurrentUserHasRightTo(eUserRights right);
	void SetRight(eUserRights right);
	bool Parse(const char *s);
	const char *ToText(void);
	bool Save(FILE *f);
};

// --- cUsers --------------------------------------------------------
class cUsers : public cConfig<cUser> {
  public:
	bool ValidUserLogin(const std::string& login, const std::string& password);
	bool ValidLogin(const std::string& login, const std::string& password);
    bool Delete(const std::string& Name);
	static cUser* GetByUserId(const std::string& Id);
	static cUser* GetByUserName(const std::string& Name);
	int GetNewId();

	static std::string logged_in_user;
};

extern cUsers Users;

}

#endif
