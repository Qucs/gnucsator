#!/usr/bin/env python3

# (c) 2020 Daniel Mulholland
# License: "In case it's of any use to someone else I include it below" [1]

# faster postprocessing script, replace
# [ -f $out.tr ] && cat $out.tr | postproc >> $outfile
# with a reference to the python script
# [ -f $out.tr ] && cat $out.tr | qucs_data_convert.py $out.tr $outfile

import sys
from pathlib import Path

import pandas as pd

SPACE = '  '

input_file = Path(sys.argv[1])
output_file = Path(sys.argv[2])

df = pd.read_csv(input_file, sep='\s+', header=0)

print('Reading from file: ' + str(input_file))

output_file_csv = output_file
df.to_csv(output_file.parent / Path(output_file.stem + '.csv'))

dep_variables = [v for v in list(df.columns.values) if v != '#Time']

print('Variables: ' + ",".join(dep_variables))

def write_values(dataset, type='indep', name='time', datasetname='#Time'):
  output = [ ]

  if name != 'time':
    output.append('<' + type + ' ' + name + ' ' + 'time' + '>')
  else:
    output.append('<' + type + ' ' + name + ' ' + str(len(df[datasetname]) - 1) + '>')

  for r in dataset[datasetname].iteritems():
    output.append(SPACE + str(r[1]))

  output.append('</' + type + '>')
  output.append('')
  return "\n".join(output)

print('Writing to: ' + str(output_file))

with open(output_file, 'w') as f:
  f.write('<Qucs Dataset >')
  f.write('\n')
  f.write(write_values(df, 'indep', 'time', '#Time'))
  for v in dep_variables:
    f.write(write_values(df, 'dep', v, v))


# [1] https://github.com/Qucs/qucs/issues/986
