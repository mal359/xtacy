/*
 *  Look like you're playing tetris .. try to make it smart
 *
 */

static int ***tetrisboard;
static int inited=0;

void
initTetris ()
{
  int i,j,k;
  tetrisboard=(int ***)malloc(options.windows*sizeof(int**));
  for(i=0;i<options.windows;i++)
  {
    tetrisboard[i]=(int**)malloc(20*sizeof(int*));
    for(j=0;j<20;j++)
    {
      tetrisboard[i][j]=(int*)calloc(10,sizeof(int));
    }
  } 
  inited=1;
}

void
drawTetris(int winno)
{

}

void
checkForLines(int winno)
{
  int i,j;

  for(i=0;i<20;i++)
  {
    for(j=0;j<10;j++)
    {
      if(tetrisboard[winno][i][j]==0)
        break;
    }
    if(j==10)
    {
/* a whole line.. coool.  remove it */
       removeLine(winno,i);
    }
  }

}

void
dropPiece(int winno)
{

}

void
removeLine(int winno,int line)
{
/* blank out the line */
/* move everything above it down one line */
}

