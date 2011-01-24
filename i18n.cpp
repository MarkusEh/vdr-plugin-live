/*
	This file has some own functionality and is used as backward
	compatibility for vdr prior to version 1.5.7 language support.
 */

#include "i18n.h"

namespace vdrlive {

I18n& LiveI18n()
{
    static I18n instance;
    return instance;
}

I18n::I18n()
  : m_encoding(
#if VDRVERSNUM >= 10503
        cCharSetConv::SystemCharacterTable() ? cCharSetConv::SystemCharacterTable() : "UTF-8"
#else
        I18nCharSets()[::Setup.OSDLanguage]
#endif
        )
{
    // fix encoding spelling for html standard.
    std::string const iso("iso");
    if (m_encoding.find(iso) != std::string::npos) {
        if (iso.length() == m_encoding.find_first_of("0123456789")) {
            m_encoding.insert(iso.length(), "-");
        }
    }
}

#if VDRVERSNUM < 10507
#include "i18n-generated.h"
#endif

#ifdef NEVER
const tI18nPhrase Phrases[] = {
    { "Live Interactive VDR Environment",
      "Live Interactive VDR Environment",
      "", // Slovenski
      "", // Italiono
      "", // Nederlands
      "", // Português
      "Environnement interactif Live VDR", // Français Patrice Staudt 16.06.2007
      "", // Norsk
      "Live-integroitu VDR-ympäristö",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "What's running at",
      "Was läuft um",
      "", // Slovenski
      "", // Italiono
      "", // Nederlands
      "", // Português
      "Qu'y a t'il au programme vers", // Français
      "", // Norsk
      "Menossa kello",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "%I:%M %p", // Time formatting string (Hour:Minute suffix)
      "%H:%M Uhr", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "%H:%M", // Français
      "", // Norsk
      "%H:%M",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "%I:%M:%S %p", // Time formatting string (Hour:Minute:Seconds suffix)
      "%H:%M:%S Uhr", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "%H:%M:%S", // Français
      "", // Norsk
      "%H:%M:%S",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "%a, %b %d", // English
      "%a, %d.%m.", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "%a, %d.%m.", // Français
      "", // Norsk
      "%a, %d.%m.",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "%A, %b %d %Y", // English
      "%A, %d.%m.%Y", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "%A, %d.%m.%Y", // Français
      "", // Norsk
      "%A, %d.%m.%Y",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "%b %d %y", // English
      "%d.%m.%y", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "%d.%m.%y", // Français
      "", // Norsk
      "%d.%m.%y",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "%A, %x", // English
      "%A, %x", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "%A, %x", // Français
      "", // Norsk
      "%A, %x",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Date", // English
      "Datum", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Date", // Français
      "", // Norsk
      "Päivämäärä",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Time", // English
      "Zeit", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Temps", // Français
      "", // Norsk
      "Kellonaika",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Recordings", // English
      "Aufnahmen", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Enregistrements", // Français
      "", // Norsk
      "Tallenteet",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "List of recordings", // English
      "Liste der Aufnahmen", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Liste des enregistrements", // Français
      "", // Norsk
      "Tallennelistaus",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "No recordings found", // English
      "Keine Aufnahmen vorhanden", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Pas d'enregistrement", // Français
      "", // Norsk
      "Tallenteita ei löydy",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "playing recording", // English
      "Wiedergabe", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Lire l'enregistrement", // Français
      "", // Norsk
      "Toistetaan tallennetta",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "stop playback", // English
      "Anhalten", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "arrêter la lecture", // Français
      "", // Norsk
      "Lopeta toisto",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "resume playback", // English
      "Fortsetzen", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "continuer", // Français
      "", // Norsk
      "Jatka toistoa",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "pause playback", // English
      "Pause", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "pause", // Français
      "", // Norsk
      "Pysäytä toisto",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "fast forward", // English
      "Suchlauf vorwärts", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "avance rapide", // Français
      "", // Norsk
      "Pikakelaus eteenpäin",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "fast rewind", // English
      "Suchlauf rückwärts", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "retour rapide", // Français
      "", // Norsk
      "Pikakelaus taaksepäin",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Remote Control", // English
      "Fernbedienung", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Télécommande", // Français
      "", // Norsk
      "Kauko-ohjain",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Interval", // English
      "Intervall", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Intervalle", // Français
      "", // Norsk
      "Päivitysväli",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Save", // English
      "Speichern", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Sauvegarde", // Français
      "", // Norsk
      "Tallenna",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Menu", // English
      "Menü", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Menu", // Français
      "", // Norsk
      "Valikko",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Exit", // English
      "Zurück", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Quitter", // Français
      "", // Norsk
      "Poistu",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Back", // English
      "Zurück", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Retour", // Français
      "", // Norsk
      "Takaisin",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Ok", // English
      "Ok", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Ok", // Français
      "", // Norsk
      "Ok",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Vol+", // English
      "Laut+", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Vol+", // Français
      "", // Norsk
      "Ääni+",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Vol-", // English
      "Laut-", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Vol-", // Français
      "", // Norsk
      "Ääni-",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Mute", // English
      "Stumm", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Muet", // Français
      "", // Norsk
      "Mykistä",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "New timer", // English
      "Neuen Timer anlegen", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Nouvelle programmation", // Français
      "", // Norsk
      "Luo uusi ajastin",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Edit", // English
      "Ändern", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Changer", // Français
      "", // Norsk
      "Muokkaa",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Weekday", // English
      "Wochentag", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Jour de la semaine", // Français
      "", // Norsk
      "Viikonpäivä",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Use VPS", // English
      "VPS verwenden", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Utiliser VPS", // Français
      "", // Norsk
      "Käytä VPS-toimintoa",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Monday", // English
      "Montag", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Lundi", // Français
      "", // Norsk
      "Maanantai",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Tuesday", // English
      "Dienstag", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Mardi", // Français
      "", // Norsk
      "Tiistai",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Wednesday", // English
      "Mittwoch", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Mercredi", // Français
      "", // Norsk
      "Keskiviikko",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Thursday", // English
      "Donnerstag", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Jeudi", // Français
      "", // Norsk
      "Torstai",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Friday", // English
      "Freitag", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Vendredi", // Français
      "", // Norsk
      "Perjantai",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Saturday", // English
      "Samstag", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Samedi", // Français
      "", // Norsk
      "Lauantai",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Sunday", // English
      "Sonntag", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Dimanche", // Français
      "", // Norsk
      "Sunnuntai",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Yes", // English
      "Ja", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Oui", // Français
      "", // Norsk
      "kyllä",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "No", // English
      "Nein", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Non", // Français
      "", // Norsk
      "ei",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Searchtimers", // English
      "Suchtimer", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Recherche", // Français
      "", // Norsk
      "Hakuajastimet",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "retrieving status ...", // English
      "Hole Status ...", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "mise à jours de l'état", // Français
      "", // Norsk
      "Haetaan tietoja ...",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "previous channel", // English
      "Sender zurück", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "chaîne précédent", // Français
      "", // Norsk
      "Edellinen kanava",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "next channel", // English
      "Sender vor", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Chaîne suivante", // Français
      "", // Norsk
      "Seuraava kanava",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Record this", // English
      "Diese Sendung aufnehmen", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Enregistrer cette émission", // Français
      "", // Norsk
      "Tallenna ohjelma",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Edit this", // English
      "Timer editieren", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Changer cette programmation", // Français
      "", // Norsk
      "Muokkaa ajastinta",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Click to view details.", // English
      "Für Details klicken.", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Clic pour voire les détails.", // Français
      "", // Norsk
      "Napsauta katsoaksesi lisätietoja.",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "more", // English
      "mehr", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "plus", // Français
      "", // Norsk
      "lisätietoja",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Switch to this channel.", // English
      "Zu diesem Kanal umschalten.", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Changer vers cette chaîne. ", // Français
      "", // Norsk
      "Vaihda kanavalle",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "play this recording.", // English
      "Diese Aufnahme abspielen.", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "lire cette enregistrement.", // Français
      "", // Norsk
      "Toista tallenne",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Toggle updates on/off.", // English
      "Statusabfrage ein- oder ausschalten.", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Activer/Désactiver l'update du status", // Français
      "", // Norsk
      "Aseta tilannekysely päälle/pois",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "no epg info for current event!", // English
      "Keine Infos zur Sendung!", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Pas d'infos pour l'émission!", // Français
      "", // Norsk
      "Lähetyksellä ei ole ohjelmatietoja!",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "no epg info for current channel!", // English
      "Dieser Kanal hat kein EPG!", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Cette chaîne n'a pas d'EPG!", // Français
      "", // Norsk
      "Kanavalla ei ole ohjelmatietoja!",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "no current channel!", // English
      "Keinen Kanal gefunden!", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "pas de chaîne trouvé!", // Français
      "", // Norsk
      "Kanavaa ei löydy!",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "error retrieving status info!", // English
      "Fehler: Status nicht verfügbar!", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "erreur: pas d'information d'état!", // Français
      "", // Norsk
      "Virhe: tilannetietoja ei saatavilla!",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "No server response!", // English
      "Der Server antwortet nicht!", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "Palvelin ei vastaa!", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Failed to update infobox!", // English
      "Kann Infobox nicht aktualisieren!", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "Infolaatikon päivitys epäonnistui!", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "loading data", // English
      "Daten nachladen", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "ladataan tietója", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "an error occured!", // English
      "Es ist ein Fehler aufgetreten!", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "virhe havaittu!", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Request succeeded!", // English
      "Aktion durchgeführt!", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Request failed!", // English
      "Aktion fehlgeschlagen!", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Show live logo image", // English
      "Zeige das Live Logo", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "Näytä Live-logo", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Use ajax technology", // English
      "Verwende AJAX Technologie", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "Käytä AJAX-tekniikkaa", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Show dynamic VDR information box", // English
      "Zeige dynamische VDR Status Box", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "Näytä dynaaminen VDR:n infolaatikko", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "View the schedule of this channel", // English
      "Zeige Programm dieses Kanals", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Regarder le programme de cette chaîne", // Français
      "", // Norsk
      "Näytä ohjelmisto kanavalta",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "No schedules available for this channel", // English
      "Für diesen Kanal liegen keine EPG-Informationen vor", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "Tälle kanavalle ei ole saatavilla ohjelmistoa", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Search term", // English
      "Suchbegriff", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Mot clé", // Français
      "", // Norsk
      "Hakuehto",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Search mode", // English
      "Suchmodus", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Mode de recherche", // Français
      "", // Norsk
      "Hakutapa",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "phrase",
      "Ausdruck",
      "",// TODO
      "frase",// Italiano
      "uitdruk",
      "",// TODO
      "Phrase",
      "",// TODO
      "fraasi",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "all words",
      "alle Worte",
      "",// TODO
      "tutte le parole",// Italiano
      "alle woorden",
      "",// TODO
      "tout les mots",
      "",// TODO
      "kaikki sanat",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "at least one word",
      "ein Wort",
      "",// TODO
      "almeno una parola",// Italiano
      "ten minste een woord",
      "",// TODO
      "un mot",
      "",// TODO
      "yksi sana",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },

    { "match exactly",
      "exakt",
      "",// TODO
      "esatta corrispondenza",// Italiano
      "precies passend",
      "",// TODO
      "correspond exactement",
      "",// TODO
      "täsmällinen",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Ees  ti
      "",// TODO Dansk
      "",// TODO Èesky (Czec  h)

    },
    { "regular expression",
      "regulärer Ausdruck",
      "",// TODO
      "espressione regolare",// Italiano
      "reguliere uitdruk  king",
      "",// TODO
      "expression réguliere",
      "",// TODO
      "säännöllinen lauseke",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Ees  ti
      "",// TODO Dansk
      "",// TODO Èesky (Czec  h)
    },
    { "Match case",
      "Groß/klein",
      "",// TODO
      "Mai    uscolo/Minuscolo",// Italiano
      "Idem case",
      "",// TODO
      "Maj/Minuscule",
      "",// TODO
      "Huomioi kirjainkoko",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Tolerance",
      "Toleranz",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "Tolérance",// TODO
      "",// TODO
      "Toleranssi",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "fuzzy",
      "unscharf",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "imprécis",
      "",// TODO
      "sumea",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Search in", // English
      "Suche in", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Recherche dans", // Français
      "", // Norsk
      "Hae kentistä",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Title", // English
      "Titel", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Titre", // Français
      "", // Norsk
      "Otsikko",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Episode", // English
      "Episode", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Épisode", // Français
      "", // Norsk
      "Jakson nimi",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Description", // English
      "Beschreibung", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Description", // Français
      "", // Norsk
      "Kuvaus",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Show schedule of channel", // English
      "Zeige Programm dieses Kanals", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Montrer le programme de la chaîne", // Français
      "", // Norsk
      "Näytä kanavan ohjelmisto",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Use channel",
      "Verw. Kanal",
      "",// TODO
      "Utilizzare canale",// Italiano
      "Gebruik kanaal",
      "",// TODO
      "Utiliser la chaîne",
      "",// TODO
      "Käytä kanavaa",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "interval",
      "Bereich",
      "",// TODO
      "intervallo",// Italiano
      "interval",
      "",// TODO
      "intervalle",// Francais Pat
      "",// TODO
      "kyllä",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "channel group",
      "Kanalgruppe",
      "",// TODO
      "gruppo canali",// Italiano
      "kanaal groep",
      "",// TODO
      "Groupe de chaînes",// Francais Pat
      "",// TODO
      "kanavaryhmä",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "only FTA",
      "ohne PayTV",
      "",// TODO
      "solo FTA",// Italiano
      "alleen FTA",
      "",// TODO
      "sans TV-Payante",
      "",// TODO
      "vapaat kanavat",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "from channel",
      "von Kanal",
      "",// TODO
      "da canale",// Italiano
      "van kanaal",
      "",// TODO
      "de la chaîne",
      "",// TODO
      "Kanavasta",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "to channel",
      "bis Kanal",
      "",// TODO
      "a canale",// Italiano
      "tot kanaal",
      "",// TODO
      "à la chaîne",
      "",// TODO
      "Kanavaan",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Use extended EPG info",
      "Verw. erweiterte EPG Info",
      "",// TODO
      "Utilizzare informazioni EPG estesa",// Italiano
      "Gebruik uitgebreide EPG info",
      "",// TODO
      "Utiliser les infos EPG avancées",// Francais Pat
      "",// TODO
      "Käytä laajennettua ohjelmaopasta",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Use time",
      "Verw. Uhrzeit",
      "",// TODO
      "Utilizzare l'orario",// Italiano
      "Gebruik tijd",
      "",// TODO
      "Utiliser l'heure",
      "",// TODO
      "Käytä aloitusaikaa",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Start after",
      "Start nach",
      "",// TODO
      "Comincia dopo",// Italiano
      "Start na",
      "",// TODO
      "Départ après",
      "",// TODO
      "Aloitusaika aikaisintaan",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Start before",
      "Start vor",
      "",// TODO
      "Comincia prima",// Italiano
      "Start voor",
      "",// TODO
      "Départ avant",
      "",// TODO
      "Aloitusaika viimeistään",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Use duration",
      "Verw. Dauer",
      "",// TODO
      "Utilizzare durata",// Italiano
      "Gebruiks duur",
      "",// TODO
      "Durée d'utilisation",
      "",// TODO
      "Käytä kestoaikaa",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Max. duration",
      "Max. Dauer",
      "",// TODO
      "Durata Massima",// Italiano
      "Max. duur",
      "",// TODO
      "Durée max.",
      "",// TODO
      "Kestoaika enintään",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Min. duration",
      "Min. Dauer",
      "",// TODO
      "Durata Minima",// Italiano
      "Min. duur",
      "",// TODO
      "Durée min.",
      "",// TODO
      "Kestoaika vähintään",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Use day of week",
      "Verw. Wochentag",
      "",// TODO
      "Utilizzare giorno della settimana",// Italiano
      "Gebruik dag van de week",
      "",// TODO
      "Utiliser les jours de la semaine",
      "",// TODO
      "Käytä viikonpäivää",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Use blacklists",
      "Verw. Ausschlusslisten",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "Utiliser la liste des exclus",
      "",// TODO
      "Käytä mustia listoja",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Use in favorites menu",
      "In Favoritenmenü verw.",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "Utiliser dans le menu favoris",
      "",// TODO
      "Käytä suosikkina",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Use as search timer",
      "Als Suchtimer verwenden",
      "",// TODO
      "Utilizzare come timer di ricerca",// Italiano
      "Gebruik als zoek timer",
      "",// TODO
      "Utiliser la recherche",
      "",// TODO
      "Käytä hakuajastimena",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Record",
      "Aufnehmen",
      "Posnemi",
      "Registra",
      "Opnemen",
      "Gravar",
      "Enregistre",
      "Ta opp",
      "Tallenna",
      "Nagraj",
      "Grabar",
      "ÅããñáöÞ",
      "Inspelning",
      "Înregistr.",
      "Felvenni",
      "Gravar",
      "·ÐßØáì",
      "Snimi",
      "Salvesta",
      "Optag",
      "Nahrát",
    },
    { "Switch only",
      "Nur umschalten",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "Seulement changer de chaine",
      "",// TODO
      "Kanavanvaihto",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Announce only",
      "Nur ankündigen",
      "",// TODO
      "Solo annuncio (niente timer)",// Italiano
      "Alleen aankondigen (geen timer)",
      "",// TODO
      "Annoncer seulement le début de l'émission",
      "",// TODO
      "Muistutus",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Series recording",
      "Serienaufnahme",
      "",// TODO
      "Registrazione serie",// Italiano
      "Serie's opnemen",
      "",// TODO
      "Enregistrement de série",
      "",// TODO
      "Sarjatallennus",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Directory",
      "Verzeichnis",
      "",// TODO
      "Cartella",// Italiano
      "Directory",
      "",// TODO
      "Dossier",
      "",// TODO
      "Hakemisto",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Delete recordings after ... days",
      "Aufn. nach ... Tagen löschen",
      "",// TODO
      "",// Italiano
      "",// TODO
      "",// TODO
      "Effacer l'enregistrement après ... jours",// Francais Pat
      "",// TODO
      "Poista tallenteet ... päivän jälkeen",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Keep ... recordings",
      "Behalte ... Aufnahmen",
      "",// TODO
      "",// Italiano
      "",// TODO
      "",// TODO
      "Garder .... les enregistrements",
      "",// TODO
      "Säilytä ... tallennetta",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Pause when ... recordings exist",
      "Pause, wenn ... Aufnahmen exist.",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "Pause, lorsque ... l'enregistrement existe.",
      "",// TODO
      "Keskeytä ... tallenteen jälkeen",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Switch ... minutes before start",
      "Umschalten ... Minuten vor Start",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "Changer ... minutes avant le début",
      "",// TODO
      "Vaihda ... minuuttia ennen alkua",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Avoid repeats",
      "Vermeide Wiederholung",
      "",// TODO
      "",// Italiano
      "",// TODO
      "",// TODO
      "Eviter les répétitions",// Francais Pat
      "",// TODO
      "Estä uusinnat",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Allowed repeats",
      "Erlaubte Wiederholungen",
      "",// TODO
      "",// Italiano
      "",// TODO
      "",// TODO
      "Répétitions autorisées",// Francais Pat
      "",// TODO
      "Sallittujen uusintojen lukumäärä",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Only repeats within ... days",
      "Nur Wiederh. innerhalb ... Tagen",
      "",// TODO
      "",// Italiano
      "",// TODO
      "",// TODO
      "Que répétition, pendant ... jours",// Francais Pat
      "",// TODO
      "Vain uusinnat ... päivän sisällä",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Compare title",
      "Vergleiche Titel",
      "",// TODO
      "",// Italiano
      "",// TODO
      "",// TODO
      "Comparer titres",// Francais Pat
      "",// TODO
      "Vertaa nimeä",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Compare subtitle",
      "Vergleiche Untertitel",
      "",// TODO
      "",// Italiano
      "",// TODO
      "",// TODO
      "Comparer les sous-titres",// Francais Pat
      "",// TODO
      "Vertaa jakson nimeä",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)

    },
    { "Compare summary",
      "Vergleiche Beschreibung",
      "",// TODO
      "",// Italiano
      "",// TODO
      "",// TODO
      "Comparer les descriptions",// Francais Pat
      "",// TODO
      "Vertaa kuvausta",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Compare",
      "Vergleiche",
      "",// TODO
      "",// Italiano
      "",// TODO
      "",// TODO
      "Comparer",// Francais Pat
      "",// TODO
      "Vertaa",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Selection",
      "Auswahl",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "Selection",// Francais Pat
      "",// TODO
      "valittu",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)

    },
    { "all",
      "alle",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "tous",// Francais Pat
      "",// TODO
      "kaikki",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "Search results",
      "Suchergebnisse",
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Résultats", // Français
      "", // Norsk
      "Hakutulokset",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "No search results",
      "keine Suchergebnisse",
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "pas de résultat de recherche", // Français
      "", // Norsk
      "Ei hakutuloksia",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Power", // English
      "Ausschalten", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Éteindre", // Français
      "", // Norsk
      "Virta",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Begin Recording", // English
      "Sofortaufnahme", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Enregistrement direct", // Français
      "", // Norsk
      "Aloita tallennus",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Up", // English
      "Hoch", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Vers le haut", // Français
      "", // Norsk
      "Ylös",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Down", // English
      "Runter", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Vers le bas", // Français
      "", // Norsk
      "Alas",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Left", // English
      "Links", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Gauche", // Français
      "", // Norsk
      "Vasen",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Right", // English
      "Rechts", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Droite", // Français
      "", // Norsk
      "Oikea",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Red", // English
      "Rot", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Rouge", // Français
      "", // Norsk
      "Punainen",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Green", // English
      "Grün", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Vert", // Français
      "", // Norsk
      "Vihreä",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Yellow", // English
      "Gelb", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Jaune", // Français
      "", // Norsk
      "Keltainen",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Blue", // English
      "Blau", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Bleu", // Français
      "", // Norsk
      "Sininen",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "New",
      "Neu",
      "Novo",
      "Nuovo",
      "Nieuw",
      "Novo",
      "Nouveau",
      "Ny",
      "Uusi",
      "Nowy",
      "Nuevo",
      "NÝï",
      "Ny",
      "Nou",
      "Új",
      "Nou",
      "´ÞÑÐÒØâì",
      "Novi",
      "Uus",
      "Ny",
      "Nový",
    },
    { "Toggle timer active/inactive",
      "Timer aktiv/inaktiv schalten",
      "",
      "",
      "",
      "",
      "Programmation basculer actif/inactif",
      "",
      "Aseta ajastin päälle/pois",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
    },
    { "Delete timer",
      "Timer löschen",
      "",
      "",
      "",
      "",
      "Effacer la programmation",
      "",
      "Poista ajastin",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
    },
    { "Edit timer",
      "Timer bearbeiten",
      "",
      "",
      "",
      "",
      "Editer la programmation",
      "",
      "Muokkaa ajastinta",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
    },
    { "Toggle search timer actions (in)active",
      "Aktionen des Suchtimers (de)aktivieren",
      "",
      "",
      "",
      "",
      "Actions de la programmation de recherche (in)actives",
      "",
      "Aseta hakuajastin päälle/pois",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
    },
    { "Browse search timer results",
      "Suchtimerergebnisse betrachten",
      "",
      "",
      "",
      "",
      "Passer en revue les résultats de programmation de recherche",
      "",
      "Selaa hakutuloksia",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
    },
    { "Delete search timer",
      "Suchtimer löschen",
      "",
      "",
      "",
      "",
      "Effacer la programmation de recherche",
      "",
      "Poista hakuajastin",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
    },
    { "Expression",
      "Suchbegriff",
      "",
      "",
      "",
      "",
      "Expression de recherche",
      "",
      "Hakutermi",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
    },
    { "Edit search timer",
      "Suchtimer bearbeiten",
      "",
      "",
      "",
      "",
      "Editer l'expression de recherche",
      "",
      "Muokkaa hakuajastinta",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
    },
    { "New search timer",
      "Neuen Suchtimer anlegen",
      "",
      "",
      "",
      "",
      "Créer nouvelle programmation de recherche",
      "",
      "Luo uusi hakuajastin",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
    },
    { "Delete this search timer?", // English
      "Diesen Suchtimer löschen?", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Effacer cette programmation de recherche?", // Français
      "", // Norsk
      "Poistetaanko tämä hakuajastin?",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Cancel",
      "Abbrechen",
      "",
      "",
      "",
      "",
      "Interrompre",
      "",
      "Peru",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
    },

    { "All",
      "Alle",
      "",
      "",
      "",
      "",
      "Tout",
      "",
      "Kaikki",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
    },
    { "Test", // English
      "Testen", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Tester", // Français
      "", // Norsk
      "Testaa",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "The time the show may start at the latest", // English
      "Die Zeit, zu der die Sendung spätestens angefangen haben muss", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "L'heure lorsque l'émission doit commencer au plus tard", // Français
      "", // Norsk
      "Lähetyksen aloitusaika viimeistään",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "The time the show may start at the earliest", // English
      "Die Zeit, zu der die Sendung frühestens anfangen darf", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "L'heure lorsque l'émission doit commencer au plus tôt", // Français
      "", // Norsk
      "Lähetyksen aloitusaika aikaisintaan",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Search text too short - use anyway?",
      "Suchtext zu kurz - trotzdem verwenden?",
      "",// TODO
      "Il testo da cercare è troppo corto.  Continuare lo stesso?",// Italiano
      "Zoek tekst tekort - toch gebruiken?",
      "",// TODO
      "Texte de recherche est trop court - l'utiliser quand même?",
      "",// TODO
      "Liian suppea hakuehto - etsitäänkö silti?",
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO
      "",// TODO Eesti
      "",// TODO Dansk
      "",// TODO Èesky (Czech)
    },
    { "User", // English
      "Benutzer", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Utilisateur", // Français
      "", // Norsk
      "Käyttäjä",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Password", // English
      "Passwort", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Mot de passe", // Français
      "", // Norsk
      "Salasana",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Last channel to display", // English
      "Letzer angezeigter Kanal", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Dernière chaîne affichée", // Français
      "", // Norsk
      "Näytä viimeisenä kanava",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Use authentication", // English
      "Authentifizierung nutzen", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Utiliser l'authentification", // Français
      "", // Norsk
      "Käytä autentikointia",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "No limit", // English
      "Alle zeigen", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Afficher tout", // Français
      "", // Norsk
      "ei rajoitusta",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Admin login", // English
      "Admin Login", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Ouverture Admin", // Français
      "", // Norsk
      "Ylläpidon käyttäjätunnus",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Admin password", // English
      "Admin Passwort", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Mot de passe de l'Admin", // Français
      "", // Norsk
      "Ylläpidon salasana",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "VDR Live Login", // English
      "VDR Live Login", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Session VDR Live", // Français
      "", // Norsk
      "VDR Live - sisäänkirjautuminen",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Logout", // English
      "Abmelden", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Fin de la session", // Français
      "", // Norsk
      "Kirjaudu ulos",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Login", // English
      "Anmelden", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Session", // Français
      "", // Norsk
      "Kirjaudu sisään",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Search", // English
      "Suchen", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Recherche", // Français
      "", // Norsk
      "Etsi",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Extended search", // English
      "Erweiterte Suche", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Recherche étendue", // Français
      "", // Norsk
      "Laajennettu haku",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Search settings", // English
      "Einstellungen zur Suche", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Règlages de recherche", // Français
      "", // Norsk
      "Hakuasetukset",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "On archive DVD No.", // English
      "Auf Archiv-DVD Nr.", // Deutsch
      "", // Slovenski
      "In archivio DVD No.", // Italiano
      "", // Nederlands
      "", // Português
      "Sur le DVD archive no°", // Français
      "", // Norsk
      "Arkistointi-DVD:llä numero",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "On archive HDD No.", // English
      "Auf Archiv-HDD Nr.", // Deutsch
      "", // Slovenski
      "In archivio HDD No.", // Italiano
      "", // Nederlands
      "", // Português
      "Sur le HDD archive no°", // Français
      "", // Norsk
      "Arkistointi-HDD:llä numero",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Starts between", // English
      "Beginnt zwischen", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Départ entre", // Français
      "", // Norsk
      "Alkaa välillä",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Wrong username or password", // English
      "Falscher Benutzername oder Passwort", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Nom de l'utilisateur ou mot de passei sont erroné", // Français
      "", // Norsk
      "Väärä käyttäjätunnus tai salasana",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Required minimum version of epgsearch: ", // English
      "Benötigte Mindestversion von epgsearch: ", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Version minimum requise d'epgsearch: ", // Français
      "", // Norsk
      "Vaadittava versio EPGSearch-laajennoksesta: ",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "EPGSearch version outdated! Please update.", // English
      "EPGSearch-Version zu alt, bitte updaten!", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Version EPGSearch périmée! Mettre à jour Svp.", // Français
      "", // Norsk
      "EPGSearch-laajennos pitäisi päivittää!",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Search for repeats.", // English
      "Nach Wiederholungen suchen.", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Recherche de répétitions.", // Français
      "", // Norsk
      "Etsi toistuvat",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "What's on",
      "Was läuft",
      "", // Slovenski
      "", // Italiono
      "", // Nederlands
      "", // Português
      "Actuellement", // Français
      "", // Norsk
      "Menossa",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "What's on?",
      "Was läuft?",
      "", // Slovenski
      "", // Italiono
      "", // Nederlands
      "", // Português
      "Actuellement?", // Français
      "", // Norsk
      "Menossa?",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Now",
      "Jetzt",
      "", // Slovenski
      "", // Italiono
      "", // Nederlands
      "", // Português
      "maintenant", // Français
      "", // Norsk
      "Nyt",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Next",
      "als Nächstes",
      "", // Slovenski
      "", // Italiono
      "", // Nederlands
      "", // Português
      "prochainement", // Français
      "", // Norsk
      "Seuraavaksi",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "additional fixed times in 'What's on?'",
      "zusätzliche Zeitpunkte in 'Was läuft?'",
      "", // Slovenski
      "", // Italiono
      "", // Nederlands
      "", // Português
      "périodes fixes additionnelles dans 'actuellement?'", // Français
      "", // Norsk
      "Lisäajankohdat 'Menossa?'-sivulle",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Format is HH:MM. Separate multiple times with a semicolon",
      "Format ist HH:MM. Mehrere Zeiten durch Semikolon trennen",
      "", // Slovenski
      "", // Italiono
      "", // Nederlands
      "", // Português
      "Le format est HH:MM . Plusieurs périodes séparées avec un point-virgule", // Français
      "", // Norsk
      "Käytä HH:MM formaattia ja erota ajankohdat puolipisteellä",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "at",
      "um",
      "", // Slovenski
      "", // Italiono
      "", // Nederlands
      "", // Português
      "à", // Français
      "", // Norsk
      "kello",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Authors",
      "Autoren",
      "", // Slovenski
      "", // Italiono
      "", // Nederlands
      "", // Português
      "Auteur", // Français
      "", // Norsk
      "Tekijät",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Project leader",
      "Projektleiter",
      "", // Slovenski
      "", // Italiono
      "", // Nederlands
      "", // Português
      "Chef de projet", // Français
      "", // Norsk
      "Projektipäällikkö",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Webserver",
      "Webserver",
      "", // Slovenski
      "", // Italiono
      "", // Nederlands
      "", // Português
      "Serveur Web", // Français
      "", // Norsk
      "HTTP-palvelin",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Content",
      "Inhalte",
      "", // Slovenski
      "", // Italiono
      "", // Nederlands
      "", // Português
      "Contenu", // Français
      "", // Norsk
      "Sisältö",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Graphics",
      "Grafiken",
      "", // Slovenski
      "", // Italiono
      "", // Nederlands
      "", // Português
      "Graphiques", // Français
      "", // Norsk
      "Grafiikka",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Information",
      "Informationen",
      "", // Slovenski
      "", // Italiono
      "", // Nederlands
      "", // Português
      "Information", // Français
      "", // Norsk
      "Tietoja",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "LIVE version",
      "LIVE Version",
      "", // Slovenski
      "", // Italiono
      "", // Nederlands
      "", // Português
      "Version LIVE", // Français
      "", // Norsk
      "LIVE-versio",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "VDR version",
      "VDR Version",
      "", // Slovenski
      "", // Italiono
      "", // Nederlands
      "", // Português
      "Version VDR", // Français
      "", // Norsk
      "VDR-versio",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Features",
      "Unterstütze Plugins",
      "", // Slovenski
      "", // Italiono
      "", // Nederlands
      "", // Português
      "Soutien des plugins", // Français
      "", // Norsk
      "Tuetut laajennokset",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Homepage",
      "Homepage",
      "", // Slovenski
      "", // Italiono
      "", // Nederlands
      "", // Português
      "Page d'accueil", // Français
      "", // Norsk
      "Kotisivu",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Couldn't find timer. Maybe you mistyped your request?", // English
      "Konnte Timer nicht finden. Evtl. fehlerhafte Anforderung?", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "N'a pas pu trouver la programmation. Peut-être vous avez une erreur dans votre requête?", // Français
      "", // Norsk
      "Ajastinta ei löydy. Kirjoititko varmasti oikein?",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Couldn't find channel or no channels available. Maybe you mistyped your request?", // English
      "Konnte Kanal nicht finden oder keine Kanäle verfügbar. Ist die Anfrage korrekt?", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "N'a pas pu trouver la chaîne. Votre requête est t'elle corrête? ", // Français
      "", // Norsk
      "Kanavaa ei löydy. Kirjoititko varmasti oikein?",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Page error", // English
      "Seitenfehler", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Erreur de page", // Français
      "", // Norsk
      "Sivuvirhe",
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "If you encounter any bugs or would like to suggest new features, please use our bugtracker", // English
      "Für Fehler oder Verbesserungsvorschläge steht unser Bugtracker bereit", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Si vous rencontrez n'importe quels bogue ou voudriez suggérer de nouveaux dispositifs, employer notre bugtracker svp", // Français
      "", // Norsk
      "Voit raportoida sekä virheet että parannusehdotukset suoraan havaintotietokantaan", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Bugs and suggestions", // English
      "Fehlerberichte und Vorschläge", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Bogues et suggestions", // Français
      "", // Norsk
      "Virheraportoinnit ja parannusehdotukset", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Find more at the Internet Movie Database.", // English
      "Weitere Informationen in der Internet Movie Database.", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Trouver plus d'information du film dans la base de données film IMDB.", // Français
      "", // Norsk
      "Hae IMDB:stä", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Details view", // English
      "Ausführliche Ansicht", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Vue détailée", // Français
      "", // Norsk
      "Ruudukkonäkymä", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "List view", // English
      "Listenansicht", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Vue en liste", // Français
      "", // Norsk
      "Listanäkymä", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "No timer defined", // English
      "Keine Timer vorhanden", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Aucune programmation définie", // Français
      "", // Norsk
      "Ajastinta ei ole määritelty", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Start page", // English
      "Startseite", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Page de départ", // Français
      "", // Norsk
      "Aloitussivu", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "active", // English
      "aktiv", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "actif", // Français
      "", // Norsk
      "käytössä", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "required", // English
      "erforderlich", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "requis", // Français
      "", // Norsk
      "vaadittava", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Local net (no login required)", // English
      "Lokales Netz (keine Anmeldung notwendig)", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Réseau local (non requis)", // Français
      "", // Norsk
      "Paikallinen verkko (ei autentikointia)", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Theme", // English
      "Thema", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Thème", // Français
      "", // Norsk
      "Ulkoasu", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Please set login and password!", // English
      "Bitte Login und Passwort angeben!", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Entrée le nom d'utilisateur et le mot de passe svp!", // Français
      "", // Norsk
      "Aseta käyttäjätunnus sekä salasana!", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Please set a title for the timer!", // English
      "Bitte einen Titel für den Timer angeben!", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Veuillez indiquer un titre pour la programmation!", // Français
      "", // Norsk
      "Aseta nimi ajastimelle!", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Setup saved.", // English
      "Einstellungen gespeichert.", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Paramètre sauvegardé", // Français
      "", // Norsk
      "Asetukset tallennettu.", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Trigger search timer update", // English
      "Suchtimer-Update starten", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "Mise à jour des recherches de programmation maintenant", // Français
      "", // Norsk
      "Päivitä hakuajastimet", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Electronic program guide information", // English
      "Elektronische Programminformation", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "Ohjelmaoppaan tiedot", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Epg error", // English
      "EPG Fehler", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "Ohjelmaoppaan virhe", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
    },
    { "Auto-delete search timer", // English
      "Suchtimer automatisch löschen", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Portugu?s
      "", // Fran?ais
      "", // Norsk
      "", // Finnish
      "", // Polski
      "", // Espa?ol
      "", // Greek
      "", // Svenska
      "", // Rom?n?
      "", // Magyar
      "", // Catal?
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
      },
    { "after ... recordings", // English
      "nach ... Aufnahmen", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Portugu?s
      "", // Fran?ais
      "", // Norsk
      "", // Finnish
      "", // Polski
      "", // Espa?ol
      "", // Greek
      "", // Svenska
      "", // Rom?n?
      "", // Magyar
      "", // Catal?
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
      },
    { "after ... days after first rec.", // English
      "nach ... Tagen nach erster Aufnahme", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Portugu?s
      "", // Fran?ais
      "", // Norsk
      "", // Finnish
      "", // Polski
      "", // Espa?ol
      "", // Greek
      "", // Svenska
      "", // Rom?n?
      "", // Magyar
      "", // Catal?
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
      },
    { "user defined", // English
      "benutzer-definiert", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
      },
    { "from date", // English
      "ab Datum", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
      },
    { "to date", // English
      "bis Datum", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
      },
    { "mm/dd/yyyy", // English - Dateformat for Datepicker, use 'mm' for month, 'dd' for day, 'yyyy' for year
      "dd.mm.yyyy", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
      },
    { "January", // English
      "Januar", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
      },
    { "February", // English
      "Februar", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
      },
    { "March", // English
      "März", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
      },
    { "April", // English
      "April", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
      },
    { "May", // English
      "Mai", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
      },
    { "June", // English
      "Juni", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
      },
    { "July", // English
      "Juli", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
      },
    { "August", // English
      "August", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
      },
    { "September", // English
      "September", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
      },
    { "October", // English
      "Oktober", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
      },
    { "November", // English
      "November", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
      },
    { "December", // English
      "Dezember", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
      },
    /*
      { "", // English
      "", // Deutsch
      "", // Slovenski
      "", // Italiano
      "", // Nederlands
      "", // Português
      "", // Français
      "", // Norsk
      "", // Finnish
      "", // Polski
      "", // Español
      "", // Greek
      "", // Svenska
      "", // Românã
      "", // Magyar
      "", // Català
      "", // Russian
      "", // Hrvatski
      "", // Eesti
      "", // Dansk
      "", // Czech
      },
    */
    { 0 },
};
#endif

} // namespace vdrlive
