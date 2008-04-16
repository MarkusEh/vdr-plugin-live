<?php
class Status
{
	var $title;
	var $vers_string;
	var $vers_number;
	var $datefmt;

	function Status($title, $vname, $vnum, $datefmt)
	{
		$this->title = $title;
		$this->vers_string = $vname;
		$this->vers_number = $vnum;
		$this->datefmt = $datefmt;
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
		$this->MakeAnchor($this->images[$idx]);
	}


	function AllImg()
	{
		foreach($this->images as $img) {
			$this->MakeAnchor($img);
		}
	}

	function MakeAnchor($img)
	{
		echo "<a href=\"screenshots.php?img=$img\"><img src=\"img/${img}_thumb.jpg\" alt=\"$img\"/></a>\n";
	}

	function FullImage($img)
	{
		$img = basename($img);
		echo "<a href=\"screenshots.php\"><img src=\"img/${img}.jpg\" alt=\"img/${img}.jpg\"/ style=\"width: 770px\"></a>\n";
	}
}
?>
