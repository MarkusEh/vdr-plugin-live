    <div class="menu">
      <?php
	 $delim = false;
	 foreach ($menu->urls as $url => $name) {
		 if ($delim) {
			 echo " |\n";
		 }
		 echo "<a href=\"$url\">$name</a>";
		 $delim = true;
	 }
	echo "\n";
	?>
    </div>
