	  <p>
	    The following list contains the most important user
	    visible changes in LIVE since the release of the last
	    stable version.  Underneath the visible parts changes took
	    place too: e.g. the used Javascript is now based in large
	    parts on
	    the <a href="http://www.mootools.net/">Mootools</a>
	    framework.
	  </p>
          <ul>
	    <li>

	      The <strong>InfoBox</strong> has
	      become <strong>&ldquo;more intelligent&rdquo;</strong>,
	      e.g. a recording can be fast-forwarded, while playing it
	      over VDR TV out. And channels of the primary output
	      device can be switched with the controls in the InfoBox.
	    </li>
	    <li>
	      A <strong>favorite list</strong> found place under
	      &ldquo;whats on?&rdquo;. It is populated by
	      &ldquo;search timers&rdquo; with set option &ldquo;use
	      in favorite menu&rdquo; while creating the timer. Also
	      new is an option to display the &ldquo;whats on?&rdquo;
	      entries in a list.
	    </li>
	    <li>
	      Each schedules entry (only those with EPG data) owns a
	      button
	      <strong>&ldquo;show in browser&rdquo;</strong>: This
	      button calls the VLC plugin and shows a liveTV
	      stream. This does not work for recordings.
	    </li>
	    <li>
	      On &ldquo;schedule&rdquo; page, there is a
	      button <strong>&ldquo;show in browser&rdquo;</strong>.
	      All channels can be selected here, also those without
	      EPG data but which are present in the
	      channels.conf. Smaller TV providers or radio stations
	      are affected by this from time to time.
	    </li>
	    <li>
	      With newer versions of the Epgsearch plugin (starting from
	      epgsearch-0.9.24) there is a menu option for timers:
	      <strong>&ldquo;timer conflicts&rdquo;</strong>. This
	      option evaluates, which programmed timers could not be
	      recorded, because there are no more free DVB devices
	      available, or similar reasons. When the InfoBox is
	      switched active, a message is displayed on the current
	      page if Epgsearch detected a change of the timer
	      conflicts.
	    </li>
	    <li>
	      On the &ldquo;search timers&rdquo; page it is now
	      possible to start direct <strong>manual search timer
	      updates</strong>. Use it and Epgsearch plugin will
	      immediately update all search timers.
	    </li>
	    <li>
	      Within the &ldquo;edit search timers&rdquo; page the new
	      option &ldquo;use as search timer&rdquo; provides a
	      <strong>user-defined search</strong>. Here the user
	      determines start and stop date for Epgsearch in the
	      background. So you are able to build search timers,
	      which scan for example only during the period of the
	      autumn all transmissions of a series.
	    </li>
	    <li>
	      In <strong>&ldquo;recordings&rdquo;</strong> now you
	      will find a
	      button <strong>&ldquo;delete&rdquo;</strong>.  The
	      recording will be marked with VDRs methods as
	      deleted. That means the recording will be renamed with
	      the suffix &ldquo;.del&rdquo; and removed at the next
	      time when VDR cleans up deleted recordings.
	    </li>
	    <li>
	      You should visit &ldquo;setup&rdquo; directly after
	      updating live 0.1.0 to 0.2.0. There might
	      be <strong>settings</strong> for the new functions,
	      which you would like to <strong>examine</strong>.
	    </li>
	    <li>
	      Revised is the possibility to selected a suitable
	      <strong>&ldquo;theme&rdquo;</strong>, a small set of
	      different colored designs to select.
	    </li>
	    <li>
	      Support for https data transfer between LIVE and the
	      browser due to <strong>SSL support</strong> in Tntnet.
	    </li>
	    <li>
	      Further there are many <strong>small
	      innovations</strong> like the length of recordings in
	      minutes ', but we are sure you will discover them by
	      yourself.
	    </li>
          </ul>
