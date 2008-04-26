<?php
/**
 * class Status:
 * Contains the localized current status of the LIVE plugin.
 */
class Status
{
	public $title;
	public $vers_string;
	public $vers_number;
	public $datefmt;
	public $language;

	public function __construct($title, $vname, $vnum, $datefmt, $lang)
	{
		$this->title = $title;
		$this->vers_string = $vname;
		$this->vers_number = $vnum;
		$this->datefmt = $datefmt;
		$this->language = $lang;
	}
}

/**
 * class Menu:
 * holds the localized names and the urls to the pages of the main
 * menu.
 */
class Menu
{
	public $urls;

	public function __construct($entries)
	{
		$this->urls = $entries;
	}
}

/**
 * class Screenshots:
 * Contains the localized 'basic' names of the screenshot images and a
 * description of the single images.
 */
class Screenshots
{
	private $images;

	public function __construct($imgNames)
	{
		$this->images = $imgNames;
	}

	public function RandomImg()
	{
		$idx = array_rand($this->images);
		$this->MakeAnchor($idx);
	}


	public function AllImg()
	{
		foreach($this->images as $idx => $descr) {
			$this->MakeAnchor($idx);
		}
	}

	private function MakeAnchor($img)
	{
		echo "<a href=\"screenshots.php?img=$img\"><img src=\"img/${img}_thumb.jpg\" alt=\"$img\"/></a>\n";
	}

	public function FullImage($img)
	{
		$img = basename($img);
		echo "<a href=\"screenshots.php\"><img src=\"img/${img}.jpg\" alt=\"img/${img}.jpg\"/ style=\"width: 810px\"></a>\n";
	}

	public function ImageDescr($img, $error)
	{
		$img = basename($img);
		if (isset($this->images[$img]))
			return $this->images[$img];
		else
			return $error;
	}
}

/**
 * class SoftwareComponnent:
 * Describes one software component which has some kind of relation
 * with the LIVE plugin.
 */
class SoftwareComponent
{
	private $name;
	private $minVers;
	private $recVers;
	private $homepage;

	public function __construct($n, $m, $r, $h)
	{
		$this->name = $n;
		$this->minVers = $m;
		$this->recVers = $r;
		$this->homepage = $h;
	}

	public function Name()
	{
		return $this->name;
	}

	public function MinVersion()
	{
		return $this->minVers;
	}

	public function RecommendedVersion()
	{
		return $this->recVers;
	}

	public function Homepage()
	{
		return $this->homepage;
	}
}

class LiveSWConfig
{
	private $required;
	private $optional;
	private $plugins;

	public function __construct(array $r, array $o, array $p)
	{
		$this->required = $r;
		$this->optional = $o;
		$this->plugins = $p;
	}

	public function Required()
	{
		return $this->required;
	}

	public function Optional()
	{
		return $this->optional;
	}

	public function Plugins()
	{
		return $this->plugins;
	}
}

?>
