#ifndef CONFIRM_H
#define CONFIRM_H

#include "live.h"
#include "stringhelpers.h"
#include "recman.h"
#include "timers.h"
#include "epgsearch.h"
#include "users.h"

namespace vdrlive {

// note: list of ids must also be available in live/js/live/infowin.js, action_id
//(action_id == "del_" || action_id == "pur_" || action_id == "res_" || action_id == "det_" || action_id == "des_")
//  "del_" delete recording
//  "pur_" permanently delete recording
//  "res_" restore recording
//  "det_" delete timer
//  "des_" delete search timer
//
typedef std::string (*tConfirmationQuestion)(cSv id);
typedef int (*tPerformAction)(cSv id, std::string &message); // return 0 on success;

class cConfirm {
  public:
    const char *m_id; // like pur_  for purge_recording
    eUserRights m_user_rights;
    const char *m_headline_button;  // text, headline and button of popup
    const char *m_warning;   // text, warning in popup (can be nullptr)
    tConfirmationQuestion m_confirmation_question;
    tPerformAction m_perform_action;

    const char *get_headline_button() const {
      return tr(m_headline_button);
    }
    std::string get_question(cSv id) const {
      return m_confirmation_question(id.substr(4));
    }
    int perform_action(cSv id, std::string &message) const {
      return m_perform_action(id.substr(4), message);
    }
    bool currentUserHasRight() const {
      return cUser::CurrentUserHasRightTo(m_user_rights);
    }
};

inline bool operator< (const cConfirm &c1, const cConfirm &c2) { return cSv(c1.m_id) <  cSv(c2.m_id); }
inline bool operator< (const cConfirm &c1, cSv c2)             { return cSv(c1.m_id) <  c2; }
inline bool operator< (cSv c1            , const cConfirm &c2) { return     c1       <  cSv(c2.m_id); }
inline bool operator==(const cConfirm &c1, const cConfirm &c2) { return cSv(c1.m_id) == cSv(c2.m_id); }

inline static const cSortedVector<cConfirm, std::less<>> g_confirm_popups =
{
  { "del_", UR_DELRECS, trNOOP("Delete recording"), nullptr, &RecordingsManager_DeleteConfirmationQuestion, &RecordingsManager_DeleteRecording},
  { "res_", UR_DELRECS, trNOOP("Restore recording"), nullptr, &RecordingsManager_RestoreConfirmationQuestion, &RecordingsManager_RestoreRecording},
  { "pur_", UR_DELRECS, trNOOP("Permanently delete recording"), trNOOP("Warning: This cannot be undone!"), &RecordingsManager_PurgeConfirmationQuestion, &RecordingsManager_PurgeRecording},
  { "det_", UR_DELTIMERS, trNOOP("Delete timer"), nullptr, &TimerManager_DeleteConfirmationQuestion, &TimerManager_DeleteTimer},
  { "des_", UR_DELSTIMERS, trNOOP("Delete search timer"), nullptr, &SearchTimers_DeleteConfirmationQuestion, &SearchTimers_DeleteSearchTimer}

};

inline const cConfirm *get_confirm_popup(cSv id) {
  auto r = g_confirm_popups.find(id.substr(0,4));
  if (r != g_confirm_popups.end()) return &(*r);
  return nullptr;
}

}

#endif
