#!/bin/sh

tmpdir=${tmpdir-`pwd`/tmp}
pgbin=/usr/lib/postgresql/9.0/bin

make USE_PGXS=0 || exit 1

[ -f $tmpdir/postmaster.pid ] && PGDATA=$tmpdir $pgbin/pg_ctl stop -m immediate >/dev/null 2>&1
[ -d $tmpdir ] && rm -rf $tmpdir
mkdir $tmpdir
$pgbin/initdb -D $tmpdir

# make the current dir the $libdir
echo "dynamic_library_path='`pwd`'" >> $tmpdir/postgresql.conf
cat <<EOF >> $tmpdir/postgresql.conf
listen_addresses=''
fsync=no
shared_preload_libraries='\$libdir/auto_explain.so'

custom_variable_classes = 'auto_explain'
auto_explain.log_min_duration = '3s'
EOF

$pgbin/postgres -D $tmpdir -k $tmpdir 2>$tmpdir/server.err &

sleep 2;

export PGHOST=$tmpdir
SHAREDIR=$($pgbin/pg_config --sharedir)

if psql -v ON_ERROR_STOP postgres -c 'select 1' >/dev/null
then

which=currency
if [ -n "$2" ]
then
    which="$2"
fi

case $1 in
    
    # for later benchmarking of improvements...
    'benchmark-sql')
        echo "Running benchmark of pure-SQL CURRENCY datatype..."
        $pgbin/psql -f sql/currency_in_sql.sql postgres
        $pgbin/psql -f sql/benchmark_$which.sql postgres
        ;;

    'benchmark')
        echo "Running benchmark of custom CURRENCY datatype..."
        $pgbin/psql -f sql/currency.sql postgres
        $pgbin/psql -f sql/benchmark_$which.sql postgres
        ;;

    *)
        for test in setup currency_code currency
        do
            $pgbin/psql -a postgres < sql/$test.sql > expected/$test.testout 2>&1
            diff -F '^-- ' -u expected/$test.out expected/$test.testout && echo PASS: $test "($(wc -l expected/$test.out|cut -f1 -d\ ) lines)"
        done
        ;;

esac

else
    echo "---- pg server errors below----"
    cat $tmpdir/server.err
    echo "---- pg server errors above----"
    debug=1
fi

if [ -n "$debug" ]
then
	echo "to connect to your server:"
	echo export PGHOST=$PGHOST
	[ $(expr "$(which psql)" : "^$pgbin/psql") -eq 0 ] && echo export PATH=$pgbin:\$PATH
	echo "psql postgres"
	echo
	echo "to stop the server:"
	echo PGDATA=$tmpdir $pgbin/pg_ctl stop -m immediate
	exit
else
	export PGDATA=$tmpdir
	$pgbin/pg_ctl stop -m immediate >/dev/null 2>&1

	[ -d $tmpdir ] && rm -rf $tmpdir
fi



