#!/bin/sh

TOPDIR=${TOPDIR:-$(git rev-parse --show-toplevel)}
SRCDIR=${SRCDIR:-$TOPDIR/src}
MANDIR=${MANDIR:-$TOPDIR/doc/man}

BITCOIND=${BITCOIND:-$SRCDIR/blastd}
BITCOINCLI=${BITCOINCLI:-$SRCDIR/blast-cli}
#BITCOINTX=${BITCOINTX:-$SRCDIR/blast-tx}
BITCOINQT=${BITCOINQT:-$SRCDIR/qt/blast-qt}

[ ! -x $BITCOIND ] && echo "$BITCOIND not found or not executable." && exit 1

# The autodetected version git tag can screw up manpage output a little bit
BITCOINVER=($($BITCOINCLI --version | head -n1 | awk -F'[ -]' '{ print $6, $7 }'))

# Create a footer file with copyright content.
# This gets autodetected fine for blastd if --version-string is not set,
# but has different outcomes for blast-qt and blast-cli.
echo "[COPYRIGHT]" > footer.h2m
$BITCOIND --version | sed -n '1!p' >> footer.h2m

for cmd in $BITCOIND $BITCOINCLI $BITCOINTX $BITCOINQT; do
  cmdname="${cmd##*/}"
  help2man -N --version-string=${BITCOINVER[0]} --include=footer.h2m -o ${MANDIR}/${cmdname}.1 ${cmd}
  sed -i "s/\\\-${BITCOINVER[1]}//g" ${MANDIR}/${cmdname}.1
done

rm -f footer.h2m
