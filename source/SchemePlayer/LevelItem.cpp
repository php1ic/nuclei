#include "LevelItem.h"
#include "ActiveGraphicsItemGroup.h"
#include "GraphicsHighlightItem.h"
#include <QFontMetrics>
#include <QTextItem>
#include <QGraphicsScene>
#include <QTextDocument>
#include "custom_logger.h"

LevelItem::LevelItem()
  : ClickableItem(ClickableItem::EnergyLevelType)
{}

Energy LevelItem::energy() const
{
  return energy_;
}

QString LevelItem::energy_text() const
{
  if (etext_)
    return etext_->text();
  return "";
}

QString LevelItem::spin_text() const
{
  if (spintext_)
    return spintext_->text();
  return "";
}

double LevelItem::ypos() const
{
  return ypos_;
}

double LevelItem::bottom_ypos() const
{
  return ypos_ + 0.5 * line_->pen().widthF();
}

double LevelItem::nuc_line_width() const
{
  return etext_->boundingRect().width()
      + spintext_->boundingRect().width();
}

double LevelItem::feed_intensity_width() const
{
  if (feedintens_)
    return feedintens_->boundingRect().width();
  return 0;
}

void LevelItem::set_funky_position(double left, double right, double y)
{
  line_->setLine(left, y, right, y);
}

void LevelItem::set_funky2_position(double xe,
                                    double xspin,
                                    double y)
{
   etext_->setPos(xe, y - etext_->boundingRect().height());
   spintext_->setPos(xspin, y - etext_->boundingRect().height());
}

double LevelItem::max_y_height() const
{
  return
      qMax(etext_->boundingRect().height(),
           spintext_->boundingRect().height());
}

//void LevelItem::set_ypos(double new_ypos)
//{
//  ypos_ = new_ypos;
//}

void LevelItem::adjust_ypos(double offset)
{
  ypos_ = std::floor(ypos_ - offset) + 0.5 * line_->pen().widthF();
}

LevelItem::LevelItem(Level level,
                     SchemeVisualSettings vis,
                     QGraphicsScene *scene)
  : LevelItem()
{
  if (!level.energy().valid())
    return;

  energy_ = level.energy();

  QFontMetrics stdBoldFontMetrics(vis.stdBoldFont);

  item = new ActiveGraphicsItemGroup(this);
  item->setActiveColor(QColor(224, 186, 100, 180));

  line_ = new QGraphicsLineItem(-vis.outerGammaMargin, 0.0, vis.outerGammaMargin, 0.0, item);
  line_->setPen(vis.levelPen);
  // thick line for stable/isomeric levels
  if (level.halfLife().isStable() || level.isomerNum() > 0)
    line_->setPen(vis.stableLevelPen);

  clickarea_ = new QGraphicsRectItem(-vis.outerGammaMargin, -0.5*stdBoldFontMetrics.height(),
                                       2.0*vis.outerGammaMargin, stdBoldFontMetrics.height());
  clickarea_->setPen(Qt::NoPen);
  clickarea_->setBrush(Qt::NoBrush);

  highlighthelper_ = new GraphicsHighlightItem(-vis.outerGammaMargin, -0.5*vis.highlightWidth,
                                                 2.0*vis.outerGammaMargin, vis.highlightWidth);
  highlighthelper_->setOpacity(0.0);

  QString etext = QString::fromStdString(energy_.to_string());
  etext_ = new QGraphicsSimpleTextItem(etext, item);
  etext_->setFont(vis.stdBoldFont);
  etext_->setPos(0.0, -stdBoldFontMetrics.height());

  QString spintext = QString::fromStdString(level.spin().to_string());
  spintext_ = new QGraphicsSimpleTextItem(spintext, item);
  spintext_->setFont(vis.stdBoldFont);
  spintext_->setPos(0.0, -stdBoldFontMetrics.height());

  if (vis.parentpos != NoParent)
  {
    QString hltext = QString::fromStdString(level.halfLife().to_string());
    hltext_ = new QGraphicsSimpleTextItem(hltext, item);
    hltext_->setFont(vis.stdFont);
    hltext_->setPos(0.0, -0.5*stdBoldFontMetrics.height());
    item->addToGroup(hltext_);
  }

  item->addHighlightHelper(highlighthelper_);
  item->addToGroup(line_);
  item->addToGroup(clickarea_);
  item->addToGroup(etext_);
  item->addToGroup(spintext_);
  scene->addItem(item);

  // plot level feeding arrow if necessary
  if (level.normalizedFeedIntensity().uncertaintyType()
      != UncertainDouble::UndefinedType)
  {
    // create line
    feedarrow_ = new QGraphicsLineItem;
    feedarrow_->setPen(vis.feedArrowPen);
    scene->addItem(feedarrow_);
    // create arrow head
    QPolygonF arrowpol;
    arrowpol << QPointF(0.0, 0.0);
    arrowpol << QPointF((vis.parentpos == RightParent ? 1.0 : -1.0) * vis.feedingArrowHeadLength, 0.5*vis.feedingArrowHeadWidth);
    arrowpol << QPointF((vis.parentpos == RightParent ? 1.0 : -1.0) * vis.feedingArrowHeadLength, -0.5*vis.feedingArrowHeadWidth);
    arrowhead_ = new QGraphicsPolygonItem(arrowpol);
    arrowhead_->setBrush(QColor(feedarrow_->pen().color()));
    arrowhead_->setPen(Qt::NoPen);
    scene->addItem(arrowhead_);
    // create intensity label
    feedintens_ = new QGraphicsTextItem;
    feedintens_->setHtml(QString("%1 %").arg(QString::fromStdString(level.normalizedFeedIntensity().to_markup())));
    feedintens_->document()->setDocumentMargin(0);
    feedintens_->setFont(vis.feedIntensityFont);
    scene->addItem(feedintens_);
  }
}

double LevelItem::align(double leftlinelength,
                        double rightlinelength,
                        double arrowleft, double arrowright,
                        SchemeVisualSettings vis)
{
  QFontMetrics stdFontMetrics(vis.stdFont);
  QFontMetrics stdBoldFontMetrics(vis.stdBoldFont);

  // temporarily remove items from group (workaround)
  item->removeFromGroup(spintext_);
  item->removeFromGroup(etext_);
  item->removeFromGroup(clickarea_);
  item->removeFromGroup(line_);
  item->removeHighlightHelper(highlighthelper_);
  if (vis.parentpos != NoParent)
    item->removeFromGroup(hltext_);

  // rescale
  highlighthelper_->setRect(-leftlinelength, -0.5*vis.highlightWidth, leftlinelength+rightlinelength, vis.highlightWidth);
  line_->setLine(-leftlinelength, 0.0, rightlinelength, 0.0);
  clickarea_->setRect(-leftlinelength, -0.5*stdBoldFontMetrics.height(), leftlinelength+rightlinelength, stdBoldFontMetrics.height());
  spintext_->setPos(-leftlinelength + vis.outerLevelTextMargin, -stdBoldFontMetrics.height());
  etext_->setPos(rightlinelength - vis.outerLevelTextMargin - stdBoldFontMetrics.width(etext_->text()), -etext_->boundingRect().height());
  double levelHlPos = 0.0;
  if (vis.parentpos == RightParent && hltext_)
    levelHlPos = -leftlinelength - vis.levelToHalfLifeDistance - stdFontMetrics.width(hltext_->text());
  else
    levelHlPos = rightlinelength + vis.levelToHalfLifeDistance;

  if (vis.parentpos != NoParent)
    hltext_->setPos(levelHlPos, -0.5*stdBoldFontMetrics.height());

  // re-add items to group
  item->addHighlightHelper(highlighthelper_);
  item->addToGroup(line_);
  item->addToGroup(clickarea_);
  item->addToGroup(etext_);
  item->addToGroup(spintext_);
  item->addToGroup(hltext_);

  item->setPos(0.0, ypos_); // add 0.5*pen-width to avoid antialiasing artifacts

  if (feedarrow_)
  {
    double leftend = (vis.parentpos == RightParent) ? rightlinelength + vis.feedingArrowGap + vis.feedingArrowHeadLength : arrowleft;
    double rightend = (vis.parentpos == RightParent) ? arrowright : -leftlinelength - vis.feedingArrowGap - vis.feedingArrowHeadLength;
    double arrowY = ypos_;
    feedarrow_->setLine(leftend, arrowY, rightend, arrowY);
    arrowhead_->setPos((vis.parentpos == RightParent) ? rightlinelength + vis.feedingArrowGap : -leftlinelength - vis.feedingArrowGap, arrowY);
    feedintens_->setPos(leftend + 15.0, arrowY - feedintens_->boundingRect().height());
    return arrowY + 0.5*feedarrow_->pen().widthF();
  }
  return std::numeric_limits<double>::quiet_NaN();
}

