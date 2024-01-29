#!/bin/zsh

for f in g*pl
do
     printf "$f: "
     time perl $f
done

time ag -c '<changeset ' /tmp/h.osm
time fgrep -c '<changeset ' /tmp/h.osm
time ugrep  -c '<changeset ' /tmp/h.osm
