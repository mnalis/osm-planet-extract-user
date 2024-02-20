#!/bin/zsh

for f in g*pl
do
     printf "$f:\t "
     time perl $f
     echo
done

for f in p*pl
do
     printf "$f:\t "
     time perl $f < /tmp/h.osm
     echo
done

time ag -c '<changeset ' /tmp/h.osm
echo

time fgrep -c '<changeset ' /tmp/h.osm
echo

time ugrep  -c '<changeset ' /tmp/h.osm
echo
