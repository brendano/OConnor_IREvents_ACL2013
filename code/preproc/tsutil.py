from datetime import datetime, timedelta, date

# http://docs.python.org/2/library/datetime.html

## function takes a datetime object, returns a hash-able bucket identifier.
rollup_bucket = {
    'month': lambda dt: (dt.year, dt.month),
    'dow': lambda dt: dt.weekday(),
    'week': lambda dt: tuple(dt.isocalendar()[:2]),
    ## monday=0: http://docs.python.org/2/library/datetime.html#datetime.date.isocalendar
    'year': lambda dt: dt.year,
}

## function takes a bucket identifier, and returns the first date in its daterange.
bucket_leftdt = {
    # the 1st of the current month
    'month': lambda (y,m): datetime(y,m,1),
    # the previous monday
    'week': lambda (y,w): iso_to_gregorian(y,w,1),
    'year': lambda y: datetime(y,1,1),
}

month_labels = 'JFMAMJJASOND'
dow_labels = ['M','Tu','W','Th','F','Sa','Su']

## function takes a bucket identifier, returns a human-readable abbreviated label
bucket_label = {
    'month': lambda (y,m): month_labels[m-1],
    'week': lambda (y,w): w,
    'dow': lambda dow: dow_labels[dow],
    'year': lambda y: y,
  }

def week_startdate(d):
    year,week = rollup_bucket['week'](d)
    start_of_week = bucket_leftdt['week']((year,week))
    return start_of_week

# http://stackoverflow.com/questions/304256/whats-the-best-way-to-find-the-inverse-of-datetime-isocalendar
def iso_year_start(iso_year):
    "The gregorian calendar date of the first day of the given ISO year"
    fourth_jan = date(iso_year, 1, 4)
    delta = timedelta(fourth_jan.isoweekday()-1)
    return fourth_jan - delta 

def iso_to_gregorian(iso_year, iso_week, iso_day):
    "Gregorian calendar date for the given ISO year, week and day"
    year_start = iso_year_start(iso_year)
    year_start = datetime(year_start.year, year_start.month, year_start.day)
    return year_start + timedelta(days=iso_day-1, weeks=iso_week-1)

