file_does_not_exists=1;
header="Date(dd-mm-yy),Category,Amount,Name"

if [ -f "main.csv" ]
then
    file_does_not_exists=0;
    read -r firstline<main.csv
    if [[ $firstline != $header ]]
    then
        rm main.csv
        file_does_not_exists=1;
    fi
fi

(($file_does_not_exists==1))&&touch main.csv;(($file_does_not_exists==1))&&echo $header>>main.csv;args=("$@");IFS=",";to_insert=("${args[-4]}","${args[-3]}","${args[-2]}","${args[-1]}");echo "${to_insert[*]}">>main.csv;echo "inserts ${to_insert[*]} into the csv";

while getopts ':c:n:s:h:' OPTION; do
  case "${OPTION}" in
    c)awk -F, -v val="$OPTARG" '$2==val {sum+=$3} END {if(sum == "") {print 0} else {print sum}}' main.csv;;
    n)awk -F, -v val="$OPTARG" '$4==val {sum+=$3} END {if(sum == "") {print 0} else {print sum}}' main.csv;;
    s)sort -t, -k2,2 main.csv>out.csv;cat out.csv>main.csv;;
    h)
      echo "h ${OPTARG}"
      ;;
    ?)
      exit 1
      ;;
  esac
done
shift "$(($OPTIND -1))"
