<?php
$LiveSWConfigs = array(
/*
 *  devel currently same as '0.2.0'. See below!
	'devel' => new LiveSWConfig(
		array(),
		array(),
		array()
		),
*/
	'0.2.0' => new LiveSWConfig(
		array(
			new SoftwareComponent('VDR', '1.4.0', '&gt;= 1.4.7', 'http://www.cadsoft.de/vdr/download.htm'),
			new SoftwareComponent('Tntnet', '1.5.3', '&gt;= 1.6.1', 'http://www.tntnet.org/download.html'),
			new SoftwareComponent('Cxxtools', '1.4.3', '&gt;= 1.4.7', 'http://www.tntnet.org/download.html')
			),
		array(
			new SoftwareComponent('boost', '1.32.0', 'GCC &lt; 4.1<sup><small>*</small></sup>', 'http://www.boost.org')
			),
		array(
			new SoftwareComponent('epgsearch', '0.9.22', '&gt;= 0.9.24', 'http://winni.vdr-developer.org/epgsearch/index.html#download'),
			new SoftwareComponent('streamdev', '0.3.3', '&gt;= 0.3.4', 'http://streamdev.vdr-developer.org/')
			)
		),
	'0.1.0' => new LiveSWConfig(
		array(
			new SoftwareComponent('VDR', '1.4.0', '&lt;= 1.5.7', 'http://www.cadsoft.de/vdr/download.htm'),
			new SoftwareComponent('Tntnet', '1.5.3', '&gt;= 1.5.3', 'http://www.tntnet.org/download.html'),
			new SoftwareComponent('Cxxtools', '1.4.3', '&gt;= 1.4.3', 'http://www.tntnet.org/download.html'),
			new SoftwareComponent('boost<sup><small>*</small></sup>', '1.32.0', '', 'http://www.boost.org')
			),
		array(),
		array(
			new SoftwareComponent('epgsearch', '0.9.22', '&gt;= 0.9.22', 'http://winni.vdr-developer.org/epgsearch/index.html#download')
			)
		)
	);

$LiveSWConfigs['devel'] = $LiveSWConfigs['0.2.0'];

function SWConfTableRow($sc)
{
	echo "<tr>\n";
	echo "  <td><div class=\"withmargin\">" . $sc->Name() . "</div></td>\n";
	echo "  <td><div class=\"withmargin\">" . $sc->MinVersion() . "</div></td>\n";
	echo "  <td><div class=\"withmargin\">" . $sc->RecommendedVersion() . "</div></td>\n";
	echo "  <td><div class=\"withmargin\"><a href=\"" . $sc->Homepage() . "\">" . $sc->Homepage() . "</a></div></td>\n";
	echo "</tr>\n";
}
?>
