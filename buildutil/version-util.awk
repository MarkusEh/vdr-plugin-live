BEGIN {
  FS="|";
  init_revisions = 1;
  rev_trigger = 0;
  date_trigger = 0;
}

/= == ===marker=== == =/ {
  init_revisions = 0;
  FS=";";
  next;
}

init_revisions == 1 {
  # print "XXX " $1, $2;
  file_revs[$1] = $2;
  next;
}

/^Working file:/ {
  rev_trigger = 0;
  if (match($0, "^Working file: (.*)$", f) > 0) {
	if (f[1] in file_revs) {
	  revision = "revision " file_revs[f[1]];
	  rev_trigger = 1;
	}
  }
  # print "FFF " f[1], revision, rev_trigger;
  next;
}

rev_trigger == 1 && /^revision/ {
  if (match($0, revision) > 0) {
	# print "FOUND " revision, $0;
	rev_trigger = 0;
	date_trigger = 1;
  }
  next;
}

date_trigger == 1 {
  if (match($1, "date: (.*)", d) > 0) {
	print d[1];
  }
  date_trigger = 0;
}

{ next; }
