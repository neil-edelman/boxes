/** \see{Story}.
 @param
 @fixme
 @author
 @since
 @deprecated */
struct Story;

struct Story *Story();
void Story_(struct Story **storyptr);
char *StoryGetVar(const struct Story *story);
