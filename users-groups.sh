echo "===> Creating users and/or groups."
if ! /usr/sbin/pw groupshow unixspace >/dev/null 2>&1; then 
  echo "Creating group 'unixspace' with gid '854'." 
  /usr/sbin/pw groupadd unixspace -g 854; else echo "Using existing group 'unixspace'."
fi
if ! /usr/sbin/pw usershow unixspace >/dev/null 2>&1; then 
  echo "Creating user 'unixspace' with uid '854'." 
  /usr/sbin/pw useradd unixspace -u 854 -g 854  -c "ContextCX daemon" -d /nonexistent -s /usr/sbin/nologin 
  else 
echo "Using existing user 'unixspace'." 
fi
