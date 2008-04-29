	  <p>
	    Die folgende Liste enth&auml;lt die wichtigsten f&uuml;r
	    den Benutzer sichtbaren &Auml;nderungen an LIVE seit der
	    letzten stabilen Version. Daneben wurde auch unter der
	    Haube einiges ver&auml;ndert. Z.B. wurde auf das
	    Javascript
	    Framework <a href="http://www.mootools.net/">Mootools</a>
	    umgestellt.
	  </p>
          <ul>
	    <li>
	      Die <strong>Infobox ist noch
	      &bdquo;intelligenter&rdquo;</strong>
	      geworden. Hier&uuml;ber kann z.B. eine Aufnahme
	      vorgespult werden, welche &uuml;ber die VDR TV Ausgabe
	      abgespielt werden. Auch kann man hier&uuml;ber die
	      Programme direkt am TV umschalten.
	    </li>
	    <li>
	      Unter &bdquo;Was l&auml;uft?&rdquo; gibt es nun eine
	      <strong>Favoritenliste</strong>. Hier werden die
	      Sendungen dargestellt, die man via
	      &bdquo;Suchtimer&rdquo; suchen l&auml;sst und hier
	      dargestellt haben will. Ein Beispiel: Beim
	      &bdquo;Suchtimer-Anlegen&rdquo; gebe ich den Suchbegriff
	      &bdquo;Volksmusik&rdquo; ein und setze den Haken bei
	      &bdquo;Im Favoritenmen&uuml; verwenden&rdquo;. Bei
	      &bdquo;Was l&auml;uft?&rdquo; &rarr;
	      &bdquo;Favoriten&rdquo; werden dann alle Sendungen mit
	      Volksmusik in der Beschreibung angezeigt. Auch neu unter
	      &bdquo;Was l&auml;uft?&rdquo; ist
	      die <strong>Listenansicht</strong>.
	    </li>
	    <li>
	      Bei den einzelnen Sendungen (also nur bei Sendungen
	      f&uuml;r die EPG-Daten vorliegen) gibt es jetzt einen
	      Knopf <strong>&bdquo;Sendung im Browser
	      anzeigen&rdquo;</strong>: Dieser Knopf ruft das
	      VLC-Plugin auf und erlaubt das Streamen von LiveTV.
	    </li>
	    <li>
	      Bei dem Men&uuml; &bdquo;Programm&rdquo; gibt es bei der
	      Programmauswahl den Knopf <strong>&rdquo;Im Browser
	      anzeigen&rdquo;</strong>.  Hier k&ouml;nnen auch Sender
	      ausgew&auml;hlt werden, die keine EPG-Daten liefern,
	      aber in der channels.conf vorhanden sind. Zum Beispiel
	      kleinere TV-Sender oder Radio Stationen.
	    </li>
	    <li>
	      Mit der entsprechenden Version vom Epgsearch-Plugin (ab
	      epgsearch-0.9.24) gibt es bei Timer nun auch einen
	      Men&uuml;punkt: <strong>&bdquo;Timerkonflikte&rdquo;</strong>.
	      Dieser Men&uuml;punkt wertet aus, welche programmierte
	      Timer nicht funktionieren k&ouml;nnen, weil z.B. keine
	      Karte mehr frei ist. Wenn die Infobox aktiv ist, wird
	      eine entsprechende Einblendung angezeigt, wenn Epgsearch
	      eine &Auml;nderung an den Timerkonflikten festgestellt
	      hat.
	    </li>
	    <li>
	      Im Men&uuml;punkt &bdquo;Suchtimer&rdquo; gibt es jetzt
	      die M&ouml;glichkeit direkt ein <strong>manuelles
	      Suchtimer-Update</strong> anzusto&szlig;en. So kann man
	      direkt das Epgsearch-Plugin dazu zwingen,
	      unverz&uuml;glich ein Update durchzuf&uuml;hren.
	    </li>
	    <li>
	      Bei &bdquo;Neuen Suchtimer anlegen&rdquo; gibt es nun
	      unter &bdquo;Als Suchtimer verwenden:&rdquo; die
	      M&ouml;glichkeit eine <strong>Benutzer-definierte
	      Suche</strong> durchzuf&uuml;hren. Hier bestimmt der
	      Benutzer, ab welchem Datum und bis zu welchem Datum
	      Epgsearch im Hintergrund seiner T&auml;tigkeit nachgehen
	      soll. Somit kann man z.B. Suchtimer bauen, die z.B. nur
	      einen Zeitraum der Herbstfolgen einer Serie durchsuchen.
	    </li>
	    <li>
	      Beim
	      Men&uuml;punkt <strong>&bdquo;Aufnahmen&rdquo;</strong>
	      gibt es jetzt
	      den <strong>&bdquo;L&ouml;schen&rdquo;&ndash;Knopf</strong>
	      neben dem Namen einer Aufnahme. Die Aufnahme wird via
	      VDR als gel&ouml;scht markiert. D.h. die Aufnahme wird
	      in &bdquo;.del&rdquo; umbenannt und beim n&auml;chsten
	      Aufr&auml;umen des VDR entsorgt.
	    </li>
	    <li>
	      Die &bdquo;Einstellungen&rdquo; sollte man nach einem
	      Update von live 0.1.0 auf 0.2.0 direkt aufsuchen. Dort
	      gibt es <strong>Einstellungen f&uuml;r die neuen
	      Funktionen</strong>, die man &uuml;berpr&uuml;fen
	      sollte.
	    </li>
	    <li>
	      &Uuml;berarbeitet ist auch die M&ouml;glichkeit sich ein
	      passendes <strong>&bdquo;Thema&rdquo;</strong>
	      auszusuchen, sprich farblich unterschiedliche Designs
	      auszuw&auml;hlen.
	    </li>
	    <li>
	      Unterst&uuml;tzung f&uuml;r https Datentransfer zwischen
	      LIVE und dem Browser
	      via <strong>SSL-Unterst&uuml;tzung</strong> in tntnet.
	    </li>
	    <li>
	      Weiterhin gibt es viele <strong>kleine
	      Neuerungen</strong> wie z.B. die Anzeige der
	      Aufnahmenl&auml;nge in Minuten, die zu entdecken sich
	      lohnt :).
	    </li>
          </ul>
