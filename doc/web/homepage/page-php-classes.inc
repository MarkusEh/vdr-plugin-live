<?php
class Status
{
	var $title;
	var $vers_string;
	var $vers_number;
	var $datefmt;
	var $language;

	function Status($title, $vname, $vnum, $datefmt, $lang)
	{
		$this->title = $title;
		$this->vers_string = $vname;
		$this->vers_number = $vnum;
		$this->datefmt = $datefmt;
		$this->language = $lang;
	}
}


class Menu
{
	var $urls;

	function Menu($entries)
	{
		$this->urls = $entries;
	}
}


class Screenshots
{
	var $images;

	function Screenshots($imgNames)
	{
		$this->images = $imgNames;
	}

	function RandomImg()
	{
		$idx = array_rand($this->images);
		$this->MakeAnchor($idx);
	}


	function AllImg()
	{
		foreach($this->images as $idx => $descr) {
			$this->MakeAnchor($idx);
		}
	}

	function MakeAnchor($img)
	{
		echo "<a href=\"screenshots.php?img=$img\"><img src=\"img/${img}_thumb.jpg\" alt=\"$img\"/></a>\n";
	}

	function FullImage($img)
	{
		$img = basename($img);
		echo "<a href=\"screenshots.php\"><img src=\"img/${img}.jpg\" alt=\"img/${img}.jpg\"/ style=\"width: 810px\"></a>\n";
	}

	function ImageDescr($img, $error)
	{
		$img = basename($img);
		if (isset($this->images[$img]))
			return $this->images[$img];
		else
			return $error;
	}
}
?>
